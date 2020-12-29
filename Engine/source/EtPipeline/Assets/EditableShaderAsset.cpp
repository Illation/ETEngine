#include "stdafx.h"
#include "EditableShaderAsset.h"

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetStub.h>
#include <EtCore/FileSystem/FileUtil.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace pl {


//=======================
// Editable Shader Asset
//=======================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableShaderAsset, "editable shader asset")
		.property("use geometry", &EditableShaderAsset::m_UseGeometry)
		.property("use fragment", &EditableShaderAsset::m_UseFragment)
	END_REGISTER_CLASS_POLYMORPHIC(EditableShaderAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableShaderAsset) // force the asset class to be linked as it is only used in reflection


//-------------------------------------
// EditableShaderAsset::LoadFromMemory
//
bool EditableShaderAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// Extract the shader text from binary data
	//------------------------
	std::string shaderContent = core::FileUtil::AsText(data);
	if (shaderContent.size() == 0)
	{
		LOG("ShaderAsset::LoadFromMemory > Empty shader file!", core::LogLevel::Warning);
		return false;
	}

	// Precompile
	//--------------------
	std::string vertSource;
	std::string geoSource;
	std::string fragSource;
	if (!Precompile(shaderContent, vertSource, geoSource, fragSource))
	{
		return false;
	}

	// Compile
	//------------------
	render::T_ShaderLoc const vertexShader = render::ShaderAsset::CompileShader(vertSource, render::E_ShaderType::Vertex);

	render::T_ShaderLoc geoShader = 0;
	if (m_UseGeometry)
	{
		geoShader = render::ShaderAsset::CompileShader(geoSource, render::E_ShaderType::Geometry);
	}

	render::T_ShaderLoc fragmentShader = 0;
	if (m_UseFragment)
	{
		fragmentShader = render::ShaderAsset::CompileShader(fragSource, render::E_ShaderType::Fragment);
	}

	// Combine Shaders into a program
	//------------------

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	render::T_ShaderLoc const shaderProgram = api->CreateProgram();

	api->AttachShader(shaderProgram, vertexShader);

	if (m_UseGeometry)
	{
		api->AttachShader(shaderProgram, geoShader);
	}

	if (m_UseFragment)
	{
		api->AttachShader(shaderProgram, fragmentShader);
		api->BindFragmentDataLocation(shaderProgram, 0, "outColor");
	}

	api->LinkProgram(shaderProgram);

	// Delete shader objects now that we have a program
	api->DeleteShader(vertexShader);

	if (m_UseGeometry)
	{
		api->DeleteShader(geoShader);
	}

	if (m_UseFragment)
	{
		api->DeleteShader(fragmentShader);
	}

	// Create shader data
	SetData(new render::ShaderData(shaderProgram));
	render::ShaderData* const shaderData = GetData();

	// Extract uniform info
	//------------------
	api->SetShader(shaderData);
	render::ShaderAsset::InitUniforms(shaderData);
	render::ShaderAsset::GetAttributes(shaderProgram, shaderData->m_Attributes);

	// all done
	return true;
}

//-------------------------------------------------
// EditableShaderAsset::SetupRuntimeAssetsInternal
//
//void EditableShaderAsset::SetupRuntimeAssetsInternal()
//{
//
//}

//-------------------------------------
// EditableShaderAsset::LoadFromMemory
//
//void EditableShaderAsset::GenerateInternal(BuildConfiguration const& buildConfig)
//{
//
//}

//---------------------------------
// EditableShaderAsset::Precompile
//
// Precompile a .glsl file into multiple shader strings that can be compiled on their own
//
bool EditableShaderAsset::Precompile(std::string &shaderContent, std::string &vertSource, std::string &geoSource, std::string &fragSource)
{
	enum ParseState {
		INIT,
		VERT,
		GEO,
		FRAG
	} state = ParseState::INIT;

	std::string extractedLine;
	while (core::FileUtil::ParseLine(shaderContent, extractedLine))
	{
		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine)))
			{
				LOG(std::string("ShaderAsset::Precompile > Replacing include at '") + extractedLine + "' failed!", core::LogLevel::Warning);
				return false;
			}
		}

		//Precompile types
		switch (state)
		{
		case INIT:
			if (extractedLine.find("<VERTEX>") != std::string::npos)
			{
				state = ParseState::VERT;
			}
			if (extractedLine.find("<GEOMETRY>") != std::string::npos)
			{
				ET_ASSERT(m_UseGeometry);
				state = ParseState::GEO;
			}
			if (extractedLine.find("<FRAGMENT>") != std::string::npos)
			{
				state = ParseState::FRAG;
			}
			break;
		case VERT:
			if (extractedLine.find("</VERTEX>") != std::string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			vertSource += extractedLine;
			vertSource += "\n";
			break;
		case GEO:
			if (extractedLine.find("</GEOMETRY>") != std::string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			geoSource += extractedLine;
			geoSource += "\n";
			break;
		case FRAG:
			if (extractedLine.find("</FRAGMENT>") != std::string::npos)
			{
				state = ParseState::INIT;
				break;
			}
			else if (extractedLine.find("#disable") != std::string::npos)
			{
				ET_ASSERT(!m_UseFragment);
				state = ParseState::INIT;
				break;
			}
			fragSource += extractedLine;
			fragSource += "\n";
			break;
		}
	}

	return true;
}

//---------------------------------
// EditableShaderAsset::ReplaceInclude
//
// Include another shader asset inline
//
bool EditableShaderAsset::ReplaceInclude(std::string &line)
{
	// Get the asset ID
	uint32 firstQ = (uint32)line.find("\"");
	uint32 lastQ = (uint32)line.rfind("\"");
	if ((firstQ == std::string::npos) ||
		(lastQ == std::string::npos) ||
		lastQ <= firstQ)
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Replacing include line '") + line + "' failed", core::LogLevel::Warning);
		return false;
	}
	firstQ++;
	std::string path = line.substr(firstQ, lastQ - firstQ);
	core::HashString const assetId(core::FileUtil::ExtractName(path).c_str());

	// Get the stub asset data
	std::vector<core::I_Asset::Reference> const& refs = m_Asset->GetReferences();
	auto const foundRefIt = std::find_if(refs.cbegin(), refs.cend(), [assetId](core::I_Asset::Reference const& reference)
		{
			ET_ASSERT(reference.GetAsset() != nullptr);
			return reference.GetAsset()->GetAsset()->GetId() == assetId;
		});

	if (foundRefIt == refs.cend())
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Asset at path '") + path + "' not found in references!", core::LogLevel::Warning);
		return false;
	}

	I_AssetPtr const* const rawAssetPtr = foundRefIt->GetAsset();
	ET_ASSERT(rawAssetPtr->GetType() == rttr::type::get<core::StubData>(), "Asset reference found at path %s is not of type StubData", path);
	AssetPtr<core::StubData> stubPtr = *static_cast<AssetPtr<core::StubData> const*>(rawAssetPtr);

	// extract the shader string
	std::string shaderContent(stubPtr->GetText(), stubPtr->GetLength());
	if (shaderContent.size() == 0)
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Shader string extracted from stub data at'") + path + "' was empty!", core::LogLevel::Warning);
		return false;
	}

	// replace the original line with the included shader
	line = "";
	std::string extractedLine;
	while (core::FileUtil::ParseLine(shaderContent, extractedLine))
	{
		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine)))
			{
				LOG(std::string("ShaderAsset::ReplaceInclude > Replacing include at '") + extractedLine + "' failed!", core::LogLevel::Warning);
				return false;
			}
		}

		line += extractedLine + "\n";
	}

	// we're done
	return true;
}


} // namespace pl
} // namespace et
