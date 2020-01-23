#include "stdafx.h"
#include "Shader.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetStub.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/GlobalRenderingSystems/SharedVarController.h>


namespace et {
namespace render {


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
	Viewport::GetCurrentApiContext()->DeleteProgram(m_ShaderProgram);

	render::parameters::DestroyBlock(m_CurrentUniforms);
}


// functionality 
/////////////////

//--------------------------------
// ShaderData::CopyParameterBlock
//
render::T_ParameterBlock ShaderData::CopyParameterBlock(render::T_ConstParameterBlock const source) const
{
	render::T_ParameterBlock const ret = render::parameters::CreateBlock(m_UniformDataSize);
	render::parameters::CopyBlockData(source, ret, m_UniformDataSize);
	return ret;
}

//----------------------------------
// ShaderData::UploadParameterBlock
//
// Upload all variables in a parameter block according to the shaders layout
//
void ShaderData::UploadParameterBlock(render::T_ConstParameterBlock const block) const
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	for (render::UniformParam const& param : m_UniformLayout)
	{
		// textures are alwats updated as we are not storing their binding points at the moement 
		// #todo: this can and should be improved
		switch (param.type)
		{
		case E_ParamType::Texture2D:
		case E_ParamType::Texture3D:
		case E_ParamType::TextureCube:
		case E_ParamType::TextureShadow:
		{
			TextureData const* const texture = render::parameters::Read<TextureData const*>(block, param.offset);
			if (texture != nullptr)
			{
				T_TextureUnit const binding = api->BindTexture(texture->GetTargetType(), texture->GetLocation(), false);
				api->UploadUniform(param.location, static_cast<int32>(binding));
			}
		}
		continue;
		}

		// check difference
		if (render::parameters::Compare(block, m_CurrentUniforms, param.offset, param.type))
		{
			continue;
		}

		// upload
		switch (param.type)
		{
		case E_ParamType::Matrix4x4:
			api->UploadUniform(param.location, render::parameters::Read<mat4>(block, param.offset));
			break;

		case E_ParamType::Matrix3x3:
			api->UploadUniform(param.location, render::parameters::Read<mat3>(block, param.offset));
			break;

		case E_ParamType::Vector4:
			api->UploadUniform(param.location, render::parameters::Read<vec4>(block, param.offset));
			break;

		case E_ParamType::Vector3:
			api->UploadUniform(param.location, render::parameters::Read<vec3>(block, param.offset));
			break;

		case E_ParamType::Vector2:
			api->UploadUniform(param.location, render::parameters::Read<vec2>(block, param.offset));
			break;

		case E_ParamType::UInt:
			api->UploadUniform(param.location, render::parameters::Read<uint32>(block, param.offset));
			break;

		case E_ParamType::Int:
			api->UploadUniform(param.location, render::parameters::Read<int32>(block, param.offset));
			break;

		case E_ParamType::Float:
			api->UploadUniform(param.location, render::parameters::Read<float>(block, param.offset));
			break;

		case E_ParamType::Boolean:
			api->UploadUniform(param.location, render::parameters::Read<bool>(block, param.offset));
			break;

		default:
			ET_ASSERT(false, "Unhandled parameter type");
			break;
		}
	}

	// ensure the state is reflected
	render::parameters::CopyBlockData(block, m_CurrentUniforms, m_UniformDataSize);
}


//===================
// Shader Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_POLYMORPHIC_CLASS(ShaderAsset, "shader asset")
	END_REGISTER_POLYMORPHIC_CLASS(ShaderAsset, core::I_Asset);
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
	std::string shaderContent = core::FileUtil::AsText(data);
	if (shaderContent.size() == 0)
	{
		LOG("ShaderAsset::LoadFromMemory > Empty shader file!", core::LogLevel::Warning);
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
	InitUniforms();
	GetAttributes(shaderProgram, m_Data->m_Attributes);

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
		LOG(std::string("ShaderAsset::ReplaceInclude > Replacing include line '") + line + "' failed", core::LogLevel::Warning);
		return false;
	}
	firstQ++;
	std::string path = line.substr(firstQ, lastQ - firstQ);
	T_Hash const assetId(GetHash(core::FileUtil::ExtractName(path)));

	// Get the stub asset data
	auto const foundRefIt = std::find_if(GetReferences().cbegin(), GetReferences().cend(), [assetId](Reference const& reference)
	{
		ET_ASSERT(reference.GetAsset() != nullptr);
		return reference.GetAsset()->GetAsset()->GetId() == assetId;
	});

	if (foundRefIt == GetReferences().cend())
	{
		LOG(std::string("ShaderAsset::ReplaceInclude > Asset at path '") + path + "' not found in references!", core::LogLevel::Warning);
		return false;
	}
	I_AssetPtr const* const rawAssetPtr = foundRefIt->GetAsset();
	ET_ASSERT(rawAssetPtr->GetType() == typeid(core::StubData), "Asset reference found at path %s is not of type StubData", path);
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

//---------------------------------
// ShaderAsset::InitUniforms
//
// Extract shader uniforms from a program
//
void ShaderAsset::InitUniforms()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	// uniform blocks
	//----------------
	std::vector<std::string> blockNames = api->GetUniformBlockNames(m_Data->m_ShaderProgram);
	for (std::string const& blockName : blockNames)
	{
		m_Data->m_UniformBlocks.emplace_back(GetHash(blockName));
	}

	// hook up shared uniform variables if the shader requires it
	render::SharedVarController const& sharedVarController = RenderingSystems::Instance()->GetSharedVarController();

	T_Hash const sharedBlockId = GetHash(sharedVarController.GetBlockName());
	auto const foundBlock = std::find(m_Data->m_UniformBlocks.cbegin(), m_Data->m_UniformBlocks.cend(), sharedBlockId);

	if (foundBlock != m_Data->m_UniformBlocks.cend())
	{
		T_BlockIndex const blockIndex = static_cast<T_BlockIndex>(foundBlock - m_Data->m_UniformBlocks.cbegin());
		api->SetUniformBlockBinding(m_Data->m_ShaderProgram, blockIndex, sharedVarController.GetBufferBinding());
	}

	// get all uniforms that are contained by uniform blocsk so we can exclude them
	std::vector<int32> blockContainedUniIndices;
	for (T_BlockIndex blockIdx = 0; blockIdx < static_cast<T_BlockIndex>(m_Data->m_UniformBlocks.size()); ++blockIdx)
	{
		std::vector<int32> indicesForCurrentBlock = api->GetUniformIndicesForBlock(m_Data->m_ShaderProgram, blockIdx);

		// merge with blockContainedUniIndices
		blockContainedUniIndices.reserve(blockContainedUniIndices.size() + indicesForCurrentBlock.size());
		blockContainedUniIndices.insert(blockContainedUniIndices.end(), indicesForCurrentBlock.begin(), indicesForCurrentBlock.end());
	}

	// default uniform variables
	//-----------------------------
	int32 const count = api->GetUniformCount(m_Data->m_ShaderProgram);

	for (int32 uniIdx = 0; uniIdx < count; ++uniIdx)
	{
		// skip uniforms contained in blocks
		if (std::find(blockContainedUniIndices.cbegin(), blockContainedUniIndices.cend(), uniIdx) != blockContainedUniIndices.cend())
		{
			continue;
		}

		// get all descriptors for index (may be more than one if contained by array)
		std::vector<UniformDescriptor> unis;
		api->GetActiveUniforms(m_Data->m_ShaderProgram, static_cast<uint32>(uniIdx), unis);

		// create a layout for each
		for (UniformDescriptor const& uni : unis)
		{
			T_Hash const hash = GetHash(uni.name);

			// ensure no hash collisions
			ET_ASSERT(std::find(m_Data->m_UniformIds.cbegin(), m_Data->m_UniformIds.cend(), hash) == m_Data->m_UniformIds.cend());

			m_Data->m_UniformIds.push_back(hash);

			m_Data->m_UniformLayout.push_back(render::UniformParam());
			render::UniformParam& uniParam = m_Data->m_UniformLayout[m_Data->m_UniformLayout.size() - 1];
			uniParam.location = uni.location;
			uniParam.type = uni.type;
			uniParam.offset = m_Data->m_UniformDataSize;

			m_Data->m_UniformDataSize += render::parameters::GetSize(uni.type);
		}
	}

	// allocate parameters
	m_Data->m_CurrentUniforms = render::parameters::CreateBlock(m_Data->m_UniformDataSize);

	// init defaults
	for (render::UniformParam const& param : m_Data->m_UniformLayout)
	{
		api->PopulateUniform(m_Data->m_ShaderProgram, param.location, param.type, static_cast<void*>(m_Data->m_CurrentUniforms + param.offset));
	}
}

//---------------------------------
// ShaderAsset::GetAttributes
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


} // namespace render
} // namespace et
