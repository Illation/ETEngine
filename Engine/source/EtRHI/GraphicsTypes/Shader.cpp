#include "stdafx.h"
#include "Shader.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Content/AssetStub.h>

#include <EtRHI/Util/SharedVarInterface.h>


namespace et {
namespace rhi {


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
	ContextHolder::GetRenderDevice()->DeleteProgram(m_ShaderProgram);

	rhi::parameters::DestroyBlock(m_CurrentUniforms);
}


// functionality 
/////////////////

//--------------------------------
// ShaderData::CopyParameterBlock
//
rhi::T_ParameterBlock ShaderData::CopyParameterBlock(rhi::T_ConstParameterBlock const source) const
{
	rhi::T_ParameterBlock const ret = rhi::parameters::CreateBlock(m_UniformDataSize);
	rhi::parameters::CopyBlockData(source, ret, m_UniformDataSize);
	return ret;
}

//----------------------------------
// ShaderData::UploadParameterBlock
//
// Upload all variables in a parameter block according to the shaders layout
//
void ShaderData::UploadParameterBlock(rhi::T_ConstParameterBlock const block) const
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	for (rhi::UniformParam const& param : m_UniformLayout)
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
			TextureData const* const texture = rhi::parameters::Read<TextureData const*>(block, param.offset);
			if (texture != nullptr)
			{
				T_TextureUnit const binding = device->BindTexture(texture->GetTargetType(), texture->GetLocation(), false);
				device->UploadUniform(param.location, static_cast<int32>(binding));
			}
		}
		continue;
		}

		// check difference
		if (rhi::parameters::Compare(block, m_CurrentUniforms, param.offset, param.type))
		{
			continue;
		}

		// upload
		switch (param.type)
		{
		case E_ParamType::Matrix4x4:
			device->UploadUniform(param.location, rhi::parameters::Read<mat4>(block, param.offset));
			break;

		case E_ParamType::Matrix3x3:
			device->UploadUniform(param.location, rhi::parameters::Read<mat3>(block, param.offset));
			break;

		case E_ParamType::Vector4:
			device->UploadUniform(param.location, rhi::parameters::Read<vec4>(block, param.offset));
			break;

		case E_ParamType::Vector3:
			device->UploadUniform(param.location, rhi::parameters::Read<vec3>(block, param.offset));
			break;

		case E_ParamType::Vector2:
			device->UploadUniform(param.location, rhi::parameters::Read<vec2>(block, param.offset));
			break;

		case E_ParamType::UInt:
			device->UploadUniform(param.location, rhi::parameters::Read<uint32>(block, param.offset));
			break;

		case E_ParamType::Int:
			device->UploadUniform(param.location, rhi::parameters::Read<int32>(block, param.offset));
			break;

		case E_ParamType::Float:
			device->UploadUniform(param.location, rhi::parameters::Read<float>(block, param.offset));
			break;

		case E_ParamType::Boolean:
			device->UploadUniform(param.location, rhi::parameters::Read<bool>(block, param.offset));
			break;

		default:
			ET_ERROR("Unhandled parameter type");
			break;
		}
	}

	// ensure the state is reflected
	rhi::parameters::CopyBlockData(block, m_CurrentUniforms, m_UniformDataSize);
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
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	T_ShaderLoc shader = device->CreateShader(type);

	//error handling
	device->CompileShader(shader, shaderSourceStr);
	if (!(device->IsShaderCompiled(shader)))
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
		device->GetShaderInfo(shader, errorInfo);
		ET_ERROR("ShaderAsset::CompileShader > Compiling %s shader failed: %s", sName.c_str(), errorInfo.c_str());
	}

	return shader;
}

//-------------------------
// ShaderAsset::LinkShader
//
// Compile and link shader sources into a shader program
//
T_ShaderLoc ShaderAsset::LinkShader(std::string const& vert, std::string const& geo, std::string const& frag, I_RenderDevice* const device)
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

	T_ShaderLoc const shaderProgram = device->CreateProgram();

	device->AttachShader(shaderProgram, vertexShader);

	if (!geo.empty())
	{
		device->AttachShader(shaderProgram, geoShader);
	}

	if (!frag.empty())
	{
		device->AttachShader(shaderProgram, fragmentShader);
		device->BindFragmentDataLocation(shaderProgram, 0, "outColor");
	}

	device->LinkProgram(shaderProgram);

	// Delete shader objects now that we have a program
	device->DeleteShader(vertexShader);

	if (!geo.empty())
	{
		device->DeleteShader(geoShader);
	}

	if (!frag.empty())
	{
		device->DeleteShader(fragmentShader);
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
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	// uniform blocks
	//----------------
	std::vector<std::string> blockNames = device->GetUniformBlockNames(data->m_ShaderProgram);
	for (std::string const& blockName : blockNames)
	{
		data->m_UniformBlocks.emplace_back(GetHash(blockName));
	}

	// hook up shared uniform variables if the shader requires it, and the engine supports it (requires full rendering library)
	I_SharedVarController const* const sharedVarController = I_SharedVarController::GetGlobal();
	if (sharedVarController != nullptr)
	{
		core::HashString const sharedBlockId(sharedVarController->GetBlockName().c_str());
		auto const foundBlock = std::find(data->m_UniformBlocks.cbegin(), data->m_UniformBlocks.cend(), sharedBlockId);

		if (foundBlock != data->m_UniformBlocks.cend())
		{
			T_BlockIndex const blockIndex = static_cast<T_BlockIndex>(foundBlock - data->m_UniformBlocks.cbegin());
			device->SetUniformBlockBinding(data->m_ShaderProgram, blockIndex, sharedVarController->GetBufferBinding());
		}
	}

	// get all uniforms that are contained by uniform blocsk so we can exclude them
	std::vector<int32> blockContainedUniIndices;
	for (T_BlockIndex blockIdx = 0; blockIdx < static_cast<T_BlockIndex>(data->m_UniformBlocks.size()); ++blockIdx)
	{
		std::vector<int32> indicesForCurrentBlock = device->GetUniformIndicesForBlock(data->m_ShaderProgram, blockIdx);

		// merge with blockContainedUniIndices
		blockContainedUniIndices.reserve(blockContainedUniIndices.size() + indicesForCurrentBlock.size());
		blockContainedUniIndices.insert(blockContainedUniIndices.end(), indicesForCurrentBlock.begin(), indicesForCurrentBlock.end());
	}

	// default uniform variables
	//-----------------------------
	int32 const count = device->GetUniformCount(data->m_ShaderProgram);

	for (int32 uniIdx = 0; uniIdx < count; ++uniIdx)
	{
		// skip uniforms contained in blocks
		if (std::find(blockContainedUniIndices.cbegin(), blockContainedUniIndices.cend(), uniIdx) != blockContainedUniIndices.cend())
		{
			continue;
		}

		// get all descriptors for index (may be more than one if contained by array)
		std::vector<UniformDescriptor> unis;
		device->GetActiveUniforms(data->m_ShaderProgram, static_cast<uint32>(uniIdx), unis);

		// create a layout for each
		for (UniformDescriptor const& uni : unis)
		{
			core::HashString const hash(uni.name.c_str());

			// ensure no hash collisions
			ET_ASSERT(std::find(data->m_UniformIds.cbegin(), data->m_UniformIds.cend(), hash) == data->m_UniformIds.cend());

			data->m_UniformIds.push_back(hash);

			data->m_UniformLayout.push_back(rhi::UniformParam());
			rhi::UniformParam& uniParam = data->m_UniformLayout[data->m_UniformLayout.size() - 1];
			uniParam.location = uni.location;
			uniParam.type = uni.type;
			uniParam.offset = data->m_UniformDataSize;

			data->m_UniformDataSize += rhi::parameters::GetSize(uni.type);
		}
	}

	// allocate parameters
	data->m_CurrentUniforms = rhi::parameters::CreateBlock(data->m_UniformDataSize);

	// init defaults
	for (rhi::UniformParam const& param : data->m_UniformLayout)
	{
		device->PopulateUniform(data->m_ShaderProgram, param.location, param.type, static_cast<void*>(data->m_CurrentUniforms + param.offset));
	}
}

//---------------------------------
// ShaderAsset::GetAttributes
//
// Extract the vertex attributes from a program, provided it has a vertex shader
//
void ShaderAsset::GetAttributes(T_ShaderLoc const shaderProgram, std::vector<ShaderData::T_AttributeLocation>& attributes)
{
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();

	int32 const count = device->GetAttributeCount(shaderProgram);
	for (int32 attribIdx = 0; attribIdx < count; ++attribIdx)
	{
		AttributeDescriptor info;
		device->GetActiveAttribute(shaderProgram, static_cast<uint32>(attribIdx), info);

		attributes.emplace_back(device->GetAttributeLocation(shaderProgram, info.name), info);
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
	I_RenderDevice* const device = ContextHolder::GetRenderDevice();
	T_ShaderLoc const shaderProgram = LinkShader(vertSource, geoSource, fragSource, device);

	// Create shader data
	m_Data = new ShaderData(shaderProgram);

	// Extract uniform info
	//------------------
	device->SetShader(m_Data);
	InitUniforms(m_Data);
	GetAttributes(shaderProgram, m_Data->m_Attributes);

	// all done
	return true;
}


} // namespace rhi
} // namespace et
