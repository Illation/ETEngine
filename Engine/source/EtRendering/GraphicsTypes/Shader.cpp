#include "stdafx.h"
#include "Shader.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/AssetRegistration.h>
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
	ContextHolder::GetRenderContext()->DeleteProgram(m_ShaderProgram);

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
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

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
			ET_ERROR("Unhandled parameter type");
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
	BEGIN_REGISTER_CLASS_ASSET(ShaderData, "shader data")
	END_REGISTER_CLASS(ShaderData);

	BEGIN_REGISTER_CLASS(ShaderAsset, "shader asset")
	END_REGISTER_CLASS_POLYMORPHIC(ShaderAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(ShaderAsset) // force the shader asset class to be linked as it is only used in reflection


// static
std::string const ShaderAsset::s_MainExtension("glsl");
std::string const ShaderAsset::s_GeoExtension("geo");
std::string const ShaderAsset::s_FragExtension("frag");

//---------------------------------
// ShaderAsset::CompileShader
//
// Compile a glsl shader
//
T_ShaderLoc ShaderAsset::CompileShader(std::string const& shaderSourceStr, E_ShaderType const type)
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

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
		ET_ERROR("ShaderAsset::CompileShader > Compiling %s shader failed: %s", sName.c_str(), errorInfo.c_str());
	}

	return shader;
}

//-------------------------
// ShaderAsset::LinkShader
//
// Compile and link shader sources into a shader program
//
T_ShaderLoc ShaderAsset::LinkShader(std::string const& vert, std::string const& geo, std::string const& frag, I_GraphicsContextApi* const api)
{
	// Compile
	//------------------
	T_ShaderLoc const vertexShader = CompileShader(vert, E_ShaderType::Vertex);

	T_ShaderLoc geoShader = 0;
	if (!geo.empty())
	{
		geoShader = CompileShader(geo, E_ShaderType::Geometry);
	}

	T_ShaderLoc fragmentShader = 0;
	if (!frag.empty())
	{
		fragmentShader = CompileShader(frag, E_ShaderType::Fragment);
	}

	// Combine Shaders into a program
	//------------------

	T_ShaderLoc const shaderProgram = api->CreateProgram();

	api->AttachShader(shaderProgram, vertexShader);

	if (!geo.empty())
	{
		api->AttachShader(shaderProgram, geoShader);
	}

	if (!frag.empty())
	{
		api->AttachShader(shaderProgram, fragmentShader);
		api->BindFragmentDataLocation(shaderProgram, 0, "outColor");
	}

	api->LinkProgram(shaderProgram);

	// Delete shader objects now that we have a program
	api->DeleteShader(vertexShader);

	if (!geo.empty())
	{
		api->DeleteShader(geoShader);
	}

	if (!frag.empty())
	{
		api->DeleteShader(fragmentShader);
	}

	return shaderProgram;
}

//---------------------------------
// ShaderAsset::InitUniforms
//
// Extract shader uniforms from a program
//
void ShaderAsset::InitUniforms(ShaderData* const data)
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	// uniform blocks
	//----------------
	std::vector<std::string> blockNames = api->GetUniformBlockNames(data->m_ShaderProgram);
	for (std::string const& blockName : blockNames)
	{
		data->m_UniformBlocks.emplace_back(GetHash(blockName));
	}

	// hook up shared uniform variables if the shader requires it
	render::SharedVarController const& sharedVarController = RenderingSystems::Instance()->GetSharedVarController();

	core::HashString const sharedBlockId(sharedVarController.GetBlockName().c_str());
	auto const foundBlock = std::find(data->m_UniformBlocks.cbegin(), data->m_UniformBlocks.cend(), sharedBlockId);

	if (foundBlock != data->m_UniformBlocks.cend())
	{
		T_BlockIndex const blockIndex = static_cast<T_BlockIndex>(foundBlock - data->m_UniformBlocks.cbegin());
		api->SetUniformBlockBinding(data->m_ShaderProgram, blockIndex, sharedVarController.GetBufferBinding());
	}

	// get all uniforms that are contained by uniform blocsk so we can exclude them
	std::vector<int32> blockContainedUniIndices;
	for (T_BlockIndex blockIdx = 0; blockIdx < static_cast<T_BlockIndex>(data->m_UniformBlocks.size()); ++blockIdx)
	{
		std::vector<int32> indicesForCurrentBlock = api->GetUniformIndicesForBlock(data->m_ShaderProgram, blockIdx);

		// merge with blockContainedUniIndices
		blockContainedUniIndices.reserve(blockContainedUniIndices.size() + indicesForCurrentBlock.size());
		blockContainedUniIndices.insert(blockContainedUniIndices.end(), indicesForCurrentBlock.begin(), indicesForCurrentBlock.end());
	}

	// default uniform variables
	//-----------------------------
	int32 const count = api->GetUniformCount(data->m_ShaderProgram);

	for (int32 uniIdx = 0; uniIdx < count; ++uniIdx)
	{
		// skip uniforms contained in blocks
		if (std::find(blockContainedUniIndices.cbegin(), blockContainedUniIndices.cend(), uniIdx) != blockContainedUniIndices.cend())
		{
			continue;
		}

		// get all descriptors for index (may be more than one if contained by array)
		std::vector<UniformDescriptor> unis;
		api->GetActiveUniforms(data->m_ShaderProgram, static_cast<uint32>(uniIdx), unis);

		// create a layout for each
		for (UniformDescriptor const& uni : unis)
		{
			core::HashString const hash(uni.name.c_str());

			// ensure no hash collisions
			ET_ASSERT(std::find(data->m_UniformIds.cbegin(), data->m_UniformIds.cend(), hash) == data->m_UniformIds.cend());

			data->m_UniformIds.push_back(hash);

			data->m_UniformLayout.push_back(render::UniformParam());
			render::UniformParam& uniParam = data->m_UniformLayout[data->m_UniformLayout.size() - 1];
			uniParam.location = uni.location;
			uniParam.type = uni.type;
			uniParam.offset = data->m_UniformDataSize;

			data->m_UniformDataSize += render::parameters::GetSize(uni.type);
		}
	}

	// allocate parameters
	data->m_CurrentUniforms = render::parameters::CreateBlock(data->m_UniformDataSize);

	// init defaults
	for (render::UniformParam const& param : data->m_UniformLayout)
	{
		api->PopulateUniform(data->m_ShaderProgram, param.location, param.type, static_cast<void*>(data->m_CurrentUniforms + param.offset));
	}
}

//---------------------------------
// ShaderAsset::GetAttributes
//
// Extract the vertex attributes from a program, provided it has a vertex shader
//
void ShaderAsset::GetAttributes(T_ShaderLoc const shaderProgram, std::vector<ShaderData::T_AttributeLocation>& attributes)
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	int32 const count = api->GetAttributeCount(shaderProgram);
	for (int32 attribIdx = 0; attribIdx < count; ++attribIdx)
	{
		AttributeDescriptor info;
		api->GetActiveAttribute(shaderProgram, static_cast<uint32>(attribIdx), info);

		attributes.emplace_back(api->GetAttributeLocation(shaderProgram, info.name), info);
	}
}


//---------------------------------------------------------------------------------

//---------------------------------
// ShaderAsset::LoadFromMemory
//
// Load shader data from binary asset content
//
bool ShaderAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// Extract the shader text from binary data
	//------------------------
	std::string vertSource = core::FileUtil::AsText(data);
	ET_ASSERT(!vertSource.empty());
	
	// Load shader sources
	//---------------------
	auto extractSourceFromStub = [](I_AssetPtr const* const rawAssetPtr, std::string& source)
		{
			ET_ASSERT(rawAssetPtr->GetType() == rttr::type::get<core::StubData>(), 
				"Asset reference found at path %s is not of type StubData", 
				rawAssetPtr->GetId().ToStringDbg());

			AssetPtr<core::StubData> stubPtr = *static_cast<AssetPtr<core::StubData> const*>(rawAssetPtr);

			// extract the shader string
			source.assign(stubPtr->GetText(), stubPtr->GetLength());
			ET_ASSERT(!source.empty());
		};

	std::string geoSource;
	std::string fragSource;
	for (core::I_Asset::Reference const& ref : GetReferences())
	{
		I_AssetPtr const* const rawAssetPtr = ref.GetAsset();
		std::string const ext = core::FileUtil::ExtractExtension(rawAssetPtr->GetAsset()->GetName());
		if (ext == s_GeoExtension)
		{
			extractSourceFromStub(rawAssetPtr, geoSource);
		}
		else if (ext == s_FragExtension)
		{
			extractSourceFromStub(rawAssetPtr, fragSource);
		}
	}
	
	// Compile
	//------------------
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();
	T_ShaderLoc const shaderProgram = LinkShader(vertSource, geoSource, fragSource, api);

	// Create shader data
	m_Data = new ShaderData(shaderProgram);

	// Extract uniform info
	//------------------
	api->SetShader(m_Data);
	InitUniforms(m_Data);
	GetAttributes(shaderProgram, m_Data->m_Attributes);

	// all done
	return true;
}


} // namespace render
} // namespace et
