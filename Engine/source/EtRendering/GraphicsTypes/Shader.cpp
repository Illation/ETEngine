#include "stdafx.h"

#include "Shader.h"
#include "Uniform.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetStub.h>

#include <rttr/registration>
#include <rttr/detail/policies/ctor_policies.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/GlobalRenderingSystems/SharedVarController.h>


//===================
// Shader Data
//===================


// Construct destruct
///////////////

//---------------------------------
// ShaderData::ShaderData
//
// Construct shader data from an OpenGl program pointer
//
ShaderData::ShaderData(T_ShaderLoc const program)
	: m_ShaderProgram(program)
{ }

//---------------------------------
// ShaderData::~ShaderData
//
// Shader data destructor
//
ShaderData::~ShaderData()
{
	for (auto &uni : m_Uniforms)
	{
		delete uni.second;
	}

	Viewport::GetCurrentApiContext()->DeleteProgram(m_ShaderProgram);
}


//===================
// Shader Asset
//===================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<ShaderAsset>("shader asset")
		.constructor<ShaderAsset const&>()
		.constructor<>()( rttr::detail::as_object() );
	rttr::type::register_converter_func( [](ShaderAsset& shader, bool& ok) -> I_Asset*
	{
		ok = true;
		return new ShaderAsset(shader);
	});
}
DEFINE_FORCED_LINKING(ShaderAsset) // force the shader asset class to be linked as it is only used in reflection

//---------------------------------
// ShaderAsset::LoadFromMemory
//
// Load shader data from binary asset content
//
bool ShaderAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// Extract the shader text from binary data
	//------------------------
	std::string shaderContent = FileUtil::AsText(data);
	if (shaderContent.size() == 0)
	{
		LOG("ShaderAsset::LoadFromMemory > Empty shader file!", Warning);
		return false;
	}

	// Precompile
	//--------------------
	bool useGeo = false;
	bool useFrag = false;
	std::string vertSource;
	std::string geoSource;
	std::string fragSource;
	if (!Precompile(shaderContent, useGeo, useFrag, vertSource, geoSource, fragSource))
	{
		return false;
	}

	// Compile
	//------------------
	T_ShaderLoc const vertexShader = CompileShader(vertSource, E_ShaderType::Vertex);

	T_ShaderLoc geoShader = 0;
	if (useGeo)
	{
		geoShader = CompileShader(geoSource, E_ShaderType::Geometry);
	}

	T_ShaderLoc fragmentShader = 0;
	if (useFrag)
	{
		fragmentShader = CompileShader(fragSource, E_ShaderType::Fragment);
	}

	// Combine Shaders into a program
	//------------------

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	T_ShaderLoc const shaderProgram = api->CreateProgram();

	api->AttachShader(shaderProgram, vertexShader);

	if (useGeo)
	{
		api->AttachShader(shaderProgram, geoShader);
	}

	if (useFrag)
	{
		api->AttachShader(shaderProgram, fragmentShader);
		api->BindFragmentDataLocation(shaderProgram, 0, "outColor");
	}

	api->LinkProgram(shaderProgram);

	// Delete shader objects now that we have a program
	api->DeleteShader(vertexShader);

	if (useGeo)
	{
		api->DeleteShader(geoShader);
	}

	if (useFrag)
	{
		api->DeleteShader(fragmentShader);
	}

	// Create shader data
	m_Data = new ShaderData(shaderProgram);

	// Extract uniform info
	//------------------
	api->SetShader(m_Data);
	GetUniformLocations(shaderProgram, m_Data->m_Uniforms);
	GetAttributes(shaderProgram, m_Data->m_Attributes);

	// hook up shared uniform variables if the shader requires it
	render::SharedVarController const& sharedVarController = RenderingSystems::Instance()->GetSharedVarController();
	m_Data->m_SharedVarIdx = api->GetUniformBlockIndex(shaderProgram, sharedVarController.GetBlockName());
	if (api->IsBlockIndexValid(m_Data->m_SharedVarIdx))
	{
		api->SetUniformBlockBinding(shaderProgram, m_Data->m_SharedVarIdx, sharedVarController.GetBufferBinding());
	}

	// all done
	return true;
}

//---------------------------------
// ShaderAsset::CompileShader
//
// Compile a glsl shader
//
T_ShaderLoc ShaderAsset::CompileShader(std::string const& shaderSourceStr, E_ShaderType const type)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	T_ShaderLoc shader = api->CreateShader(type);

	//error handling
	api->CompileShader(shader, shaderSourceStr);
	if (!(api->IsShaderCompiled(shader)))
	{
		std::string sName;
		switch (type)
		{
		case E_ShaderType::Vertex:
			sName = "vertex";
			break;
		case E_ShaderType::Geometry:
			sName = "geometry";
			break;
		case E_ShaderType::Fragment:
			sName = "fragment";
			break;
		default:
			sName = "invalid type";
			break;
		}

		std::string errorInfo;
		api->GetShaderInfo(shader, errorInfo);
		ET_ASSERT(false, "ShaderAsset::CompileShader > Compiling %s shader failed: %s", sName.c_str(), errorInfo.c_str());
	}

	return shader;
}

//---------------------------------
// ShaderAsset::Precompile
//
// Precompile a .glsl file into multiple shader strings that can be compiled on their own
//
bool ShaderAsset::Precompile(std::string &shaderContent, 
	bool &useGeo, 
	bool &useFrag, 
	std::string &vertSource, 
	std::string &geoSource, 
	std::string &fragSource)
{
	enum ParseState {
		INIT,
		VERT,
		GEO,
		FRAG
	} state = ParseState::INIT;

	std::string extractedLine;
	while (FileUtil::ParseLine(shaderContent, extractedLine))
	{
		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine)))
			{
				LOG(std::string("ShaderAsset::Precompile > Replacing include at '") + extractedLine + "' failed!", LogLevel::Warning);
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
				useGeo = true;
				state = ParseState::GEO;
			}
			if (extractedLine.find("<FRAGMENT>") != std::string::npos)
			{
				state = ParseState::FRAG;
				useFrag = true;
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
				useFrag = false;
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
// ShaderAsset::ReplaceInclude
//
// Include another shader asset inline
//
bool ShaderAsset::ReplaceInclude(std::string &line)
{
	// Get the asset ID
	uint32 firstQ = (uint32)line.find("\"");
	uint32 lastQ = (uint32)line.rfind("\"");
	if ((firstQ == std::string::npos) ||
		(lastQ == std::string::npos) ||
		lastQ <= firstQ)
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Replacing include line '") + line + "' failed", LogLevel::Warning);
		return false;
	}
	firstQ++;
	std::string path = line.substr(firstQ, lastQ - firstQ);
	T_Hash const assetId(GetHash(FileUtil::ExtractName(path)));

	// Get the stub asset data
	auto const foundRefIt = std::find_if(GetReferences().cbegin(), GetReferences().cend(), [assetId](Reference const& reference)
	{
		ET_ASSERT(reference.GetAsset() != nullptr);
		return reference.GetAsset()->GetAsset()->GetId() == assetId;
	});

	if (foundRefIt == GetReferences().cend())
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Asset at path '") + path + "' not found in references!", LogLevel::Warning);
		return false;
	}
	I_AssetPtr const* const rawAssetPtr = foundRefIt->GetAsset();
	ET_ASSERT(rawAssetPtr->GetType() == typeid(StubData), "Asset reference found at path %s is not of type StubData", path);
	AssetPtr<StubData> stubPtr = *static_cast<AssetPtr<StubData> const*>(rawAssetPtr);

	// extract the shader string
	std::string shaderContent(stubPtr->GetText(), stubPtr->GetLength());
	if (shaderContent.size() == 0)
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Shader string extracted from stub data at'") + path + "' was empty!", LogLevel::Warning);
		return false;
	}

	// replace the original line with the included shader
	line = "";
	std::string extractedLine;
	while (FileUtil::ParseLine(shaderContent, extractedLine))
	{
		//Includes
		if (extractedLine.find("#include") != std::string::npos)
		{
			if (!(ReplaceInclude(extractedLine)))
			{
				LOG(std::string("ShaderAsset::ReplaceInclude > Replacing include at '") + extractedLine + "' failed!", LogLevel::Warning);
				return false;
			}
		}

		line += extractedLine + "\n";
	}

	// we're done
	return true;
}

//---------------------------------
// ShaderAsset::GetUniformLocations
//
// Extract shader uniforms from a program
//
void ShaderAsset::GetUniformLocations(T_ShaderLoc const shaderProgram, std::map<uint32, I_Uniform*> &uniforms)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	int32 const count = api->GetUniformCount(shaderProgram);

	for (int32 uniIdx = 0; uniIdx < count; ++uniIdx)
	{
		std::vector<I_Uniform*> unis;
		api->GetActiveUniforms(shaderProgram, static_cast<uint32>(uniIdx), unis);
		
		for (I_Uniform* const uni : unis)
		{
			T_Hash const hash = GetHash(uni->name);

			// ensure no hash collisions
			ET_ASSERT(uniforms.find(hash) == uniforms.end());

			uniforms[hash] = uni;
		}
	}
}

//---------------------------------
// ShaderAsset::GetUniformLocations
//
// Extract the vertex attributes from a program, provided it has a vertex shader
//
void ShaderAsset::GetAttributes(T_ShaderLoc const shaderProgram, std::vector<ShaderData::T_AttributeLocation>& attributes)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	int32 const count = api->GetAttributeCount(shaderProgram);
	for (int32 attribIdx = 0; attribIdx < count; ++attribIdx)
	{
		AttributeDescriptor info;
		api->GetActiveAttribute(shaderProgram, static_cast<uint32>(attribIdx), info);

		attributes.emplace_back(api->GetAttributeLocation(shaderProgram, info.name), info);
	}
}
