#include "stdafx.h"
#include "EnvironmentMap.h"

#include "Shader.h"

#include <stb/stb_image.h>

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Reflection/Registration.h>
#include <EtCore/FileSystem/FileUtil.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


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
EnvironmentMap::EnvironmentMap(TextureData* map, TextureData* irradiance, TextureData* radiance) 
	: m_Map(map)
	, m_Irradiance(irradiance)
	, m_Radiance(radiance)
{
	ET_ASSERT(m_Radiance != nullptr);
	ET_ASSERT(m_Radiance->GetNumMipLevels() > 2);
	m_NumMipMaps = m_Radiance->GetNumMipLevels() - 2;
}

//---------------------------------
// EnvironmentMap::d-tor
//
// Destroy the owned textures
//
EnvironmentMap::~EnvironmentMap()
{
	SafeDelete(m_Map);
	SafeDelete(m_Irradiance);
	SafeDelete(m_Radiance);
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
		.property("Cubemap Resolution", &EnvironmentMapAsset::m_CubemapRes)
		.property("Irradiance Resolution", &EnvironmentMapAsset::m_IrradianceRes)
		.property("Radiance Resolution", &EnvironmentMapAsset::m_RadianceRes)
	END_REGISTER_CLASS_POLYMORPHIC(EnvironmentMapAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(EnvironmentMapAsset) // force the shader class to be linked as it is only used in reflection


//---------------------------------
// EnvironmentMapAsset::LoadFromMemory
//
// Loads an equirectangular texture, converts it to a cubemap, and prefilters irradiance and radiance cubemaps for IBL
//
bool EnvironmentMapAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	ContextHolder::GetRenderContext()->SetSeamlessCubemapsEnabled(true);

	//load equirectangular texture
	//****************************
	std::string extension = core::FileUtil::ExtractExtension(GetName());
	ET_ASSERT(extension == "hdr", "Expected HDR file format!");

	stbi_set_flip_vertically_on_load(true);
	int32 width = 0;
	int32 height = 0;
	int32 channels = 0;
	float* hdrFloats = stbi_loadf_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &channels, 0);

	if (hdrFloats == nullptr)
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Failed to load hdr floats from data!", core::LogLevel::Warning);
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Image is too small to display!", core::LogLevel::Warning);
		stbi_image_free(hdrFloats);
		return false;
	}

	TextureData hdrTexture(ivec2(width, height), E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
	hdrTexture.Build((void*)hdrFloats);

	// we have our equirectangular texture on the GPU so we can clean up the load data on the CPU
	stbi_image_free(hdrFloats);
	hdrFloats = nullptr;

	TextureParameters params(false);
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
	hdrTexture.SetParameters(params);

	TextureData* const envCubemap = EquirectangularToCubeMap(&hdrTexture, m_CubemapRes);

	TextureData* irradianceMap = nullptr;
	TextureData* radianceMap = nullptr;
	PbrPrefilter::PrefilterCube(envCubemap, irradianceMap, radianceMap, m_CubemapRes, m_IrradianceRes, m_RadianceRes);

	m_Data = new EnvironmentMap(envCubemap, irradianceMap, radianceMap);

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
TextureData* EquirectangularToCubeMap(TextureData const* const equiTexture, int32 const resolution)
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	//Create framebuffer
	T_FbLoc captureFBO;
	T_RbLoc captureRBO;
	api->GenFramebuffers(1, &captureFBO);
	api->GenRenderBuffers(1, &captureRBO);

	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24, ivec2(resolution));
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24, captureRBO);

	//Preallocate memory for cubemap
	TextureData* const envCubeMap = new TextureData(E_TextureType::CubeMap, ivec2(resolution));
	envCubeMap->Build();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
	params.wrapR = E_TextureWrapMode::ClampToEdge;

	envCubeMap->SetParameters(params);

	std::vector<mat4> captureViews = CubeCaptureViews();

	//Get the shader
	AssetPtr<ShaderData> equiCubeShader = core::ResourceManager::Instance()->GetAssetData<ShaderData>(core::HashString("FwdEquiCubeShader.glsl"));

	// convert HDR equirectangular environment map to cubemap equivalent
	api->SetShader(equiCubeShader.get());
	equiCubeShader->Upload("equirectangularMap"_hash, equiTexture);
	equiCubeShader->Upload("projection"_hash, CubeCaptureProjection());

	//render the cube
	//***************

	ivec2 pos, size;
	api->GetViewport(pos, size);

	api->SetViewport(ivec2(0), ivec2(resolution));
	api->BindFramebuffer(captureFBO);
	for (uint8 face = 0; face < 6; ++face)
	{
		equiCubeShader->Upload("view"_hash, captureViews[face]);
		api->LinkCubeMapFaceToFbo2D(face, envCubeMap->GetLocation(), 0);
		api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

		RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Cube>();
	}
	api->BindFramebuffer(0);

	params.genMipMaps = true;
	envCubeMap->SetParameters(params);

	api->UnbindTexture(equiTexture->GetTargetType(), equiTexture->GetLocation());
	api->UnbindTexture(envCubeMap->GetTargetType(), envCubeMap->GetLocation());

	api->SetViewport(pos, size);

	api->DeleteRenderBuffers(1, &captureRBO);
	api->DeleteFramebuffers(1, &captureFBO);

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
