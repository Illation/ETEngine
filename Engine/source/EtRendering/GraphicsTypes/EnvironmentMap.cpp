#include "stdafx.h"
#include "EnvironmentMap.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Reflection/Registration.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/BinaryReader.h>

#include <EtRHI/GraphicsTypes/Shader.h>	
#include <EtRHI/Util/PrimitiveRenderer.h>

#include <EtRendering/GlobalRenderingSystems/PbrPrefilter.h>


namespace et {
namespace render {


//=================
// Environment Map
//=================


//---------------------------------
// EnvironmentMap::c-tor
//
// Contruct an environment map from a set of cube maps - takes ownership of the textures
//
EnvironmentMap::EnvironmentMap(rhi::TextureData* map, rhi::TextureData* irradiance, rhi::TextureData* radiance) 
	: m_Map(map)
	, m_Irradiance(irradiance)
	, m_Radiance(radiance)
{
	ET_ASSERT(m_Radiance != nullptr);
	ET_ASSERT(m_Radiance->GetNumMipLevels() > 2);
	m_NumMipMaps = m_Radiance->GetNumMipLevels() - 2;
}

//---------------------------------
// EnvironmentMap::c-tor
//
// asset pointer version in order to hold references
//
EnvironmentMap::EnvironmentMap(AssetPtr<rhi::TextureData> map, AssetPtr<rhi::TextureData> irradiance, AssetPtr<rhi::TextureData> radiance)
	: m_MapAsset(map)
	, m_IrradianceAsset(irradiance)
	, m_RadianceAsset(radiance)
{
	ET_ASSERT(m_RadianceAsset != nullptr);
	ET_ASSERT(m_RadianceAsset->GetNumMipLevels() > 2);
	m_NumMipMaps = m_RadianceAsset->GetNumMipLevels() - 2;
}

//---------------------------------
// EnvironmentMap::d-tor
//
// Destroy the owned textures
//
EnvironmentMap::~EnvironmentMap()
{
	delete m_Map;
	delete m_Irradiance;
	delete m_Radiance;
}


//=======================
// Environment Map Asset
//=======================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(EnvironmentMap, "environment map")
	END_REGISTER_CLASS(EnvironmentMap);

	BEGIN_REGISTER_CLASS(EnvironmentMapAsset, "environment map asset")
	END_REGISTER_CLASS_POLYMORPHIC(EnvironmentMapAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(EnvironmentMapAsset) // force the shader class to be linked as it is only used in reflection


std::string const EnvironmentMapAsset::s_Header("ETENV");


//---------------------------------
// EnvironmentMapAsset::LoadFromMemory
//
// Loads an equirectangular texture, converts it to a cubemap, and prefilters irradiance and radiance cubemaps for IBL
//
bool EnvironmentMapAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	core::BinaryReader reader;
	reader.Open(data);
	ET_ASSERT(reader.Exists());

	// read header
	//-------------
	if (reader.ReadString(s_Header.size()) != s_Header)
	{
		ET_WARNING("Incorrect binary environment map file header");
		return false;
	}

	std::string const writerVersion = reader.ReadNullString();
	if (writerVersion != build::Version::s_Name)
	{
		ET_TRACE_W(ET_CTX_RENDER, "Environment map was written by a different engine version: %s", writerVersion.c_str());
	}

	core::HashString const envId(reader.Read<T_Hash>());
	core::HashString const irradianceId(reader.Read<T_Hash>());
	core::HashString const radianceId(reader.Read<T_Hash>());
	reader.Close();

	AssetPtr<rhi::TextureData> map;
	AssetPtr<rhi::TextureData> irradiance;
	AssetPtr<rhi::TextureData> radiance;

	for (core::I_Asset::Reference const& ref : GetReferences())
	{
		if (ref.GetId() == envId)
		{
			map = *static_cast<AssetPtr<rhi::TextureData> const*>(ref.GetAsset());
		}
		else if (ref.GetId() == irradianceId)
		{
			irradiance = *static_cast<AssetPtr<rhi::TextureData> const*>(ref.GetAsset());
		}
		else if (ref.GetId() == radianceId)
		{
			radiance = *static_cast<AssetPtr<rhi::TextureData> const*>(ref.GetAsset());
		}
	}

	if ((map == nullptr) || (irradiance == nullptr) || (radiance == nullptr))
	{
		ET_WARNING("Failed to load all texture dependencies");
		return false;
	}

	m_Data = new EnvironmentMap(map, irradiance, radiance);

	return true;
}


//=======================================
// Environment mapping utility functions
//=======================================


//---------------------------------
// EquirectangularToCubeMap
//
// Convert an equirectangular 2D texture into a cube map
//
rhi::TextureData* EquirectangularToCubeMap(rhi::TextureData const* const equiTexture, int32 const resolution)
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	//Create framebuffer
	rhi::T_FbLoc captureFBO;
	rhi::T_RbLoc captureRBO;
	device->GenFramebuffers(1, &captureFBO);
	device->GenRenderBuffers(1, &captureRBO);

	device->BindFramebuffer(captureFBO);
	device->BindRenderbuffer(captureRBO);
	device->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24, ivec2(resolution));
	device->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24, captureRBO);

	//Preallocate memory for cubemap
	rhi::TextureData* const envCubeMap = new rhi::TextureData(rhi::E_TextureType::CubeMap, rhi::E_ColorFormat::RGB16f, ivec2(resolution));
	envCubeMap->AllocateStorage();

	rhi::TextureParameters params;
	PbrPrefilter::PopulateCubeTextureParams(params);
	params.genMipMaps = false;

	envCubeMap->SetParameters(params);

	std::vector<mat4> captureViews = CubeCaptureViews();

	//Get the shader
	AssetPtr<rhi::ShaderData> equiCubeShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/FwdEquiCubeShader.glsl"));

	// convert HDR equirectangular environment map to cubemap equivalent
	device->SetShader(equiCubeShader.get());
	equiCubeShader->Upload("equirectangularMap"_hash, equiTexture);
	equiCubeShader->Upload("projection"_hash, CubeCaptureProjection());

	//render the cube
	//***************

	ivec2 pos, size;
	device->GetViewport(pos, size);

	device->SetViewport(ivec2(0), ivec2(resolution));
	device->BindFramebuffer(captureFBO);
	for (uint8 face = 0; face < 6; ++face)
	{
		equiCubeShader->Upload("view"_hash, captureViews[face]);
		device->LinkCubeMapFaceToFbo2D(face, envCubeMap->GetLocation(), 0);
		device->Clear(rhi::E_ClearFlag::CF_Color | rhi::E_ClearFlag::CF_Depth);

		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Cube>();
	}
	device->BindFramebuffer(0);

	params.genMipMaps = true;
	envCubeMap->SetParameters(params);
	envCubeMap->GenerateMipMaps();

	device->UnbindTexture(equiTexture->GetTargetType(), equiTexture->GetLocation());
	device->UnbindTexture(envCubeMap->GetTargetType(), envCubeMap->GetLocation());

	device->SetViewport(pos, size);

	device->DeleteRenderBuffers(1, &captureRBO);
	device->DeleteFramebuffers(1, &captureFBO);

	return envCubeMap;
}

//---------------------------------
// CubeCaptureProjection
//
// Projection matrix that can see exactly one cube face if placed at the centre of the cube
//
mat4 CubeCaptureProjection()
{
	return math::perspective(math::radians(90.0f), 1.0f, 0.1f, 10.0f);
}

//---------------------------------
// CubeCaptureViews
//
// Camera transformations facing the faces of a cube map from the inside
//
std::vector<mat4> CubeCaptureViews()
{
	std::vector<mat4> ret;
	ret.push_back(math::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(math::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(math::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
	ret.push_back(math::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
	ret.push_back(math::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(math::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));
	return ret;
}


} // namespace render
} // namespace et
