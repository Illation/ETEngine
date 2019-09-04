#include "stdafx.h"
#include "EnvironmentMap.h"

#include "Shader.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/FileSystem/FileUtil.h>

#include <Engine/GraphicsHelper/PrimitiveRenderer.h>
#include <Engine/GraphicsHelper/PbrPrefilter.h>


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
	using namespace rttr;

	registration::class_<EnvironmentMapAsset>("environment map asset")
		.constructor<EnvironmentMapAsset const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("Cubemap Resolution", &EnvironmentMapAsset::m_CubemapRes)
		.property("Irradiance Resolution", &EnvironmentMapAsset::m_IrradianceRes)
		.property("Radiance Resolution", &EnvironmentMapAsset::m_RadianceRes);

	rttr::type::register_converter_func([](EnvironmentMapAsset& asset, bool& ok) -> I_Asset*
	{
		ok = true;
		return new EnvironmentMapAsset(asset);
	});
}
DEFINE_FORCED_LINKING(EnvironmentMapAsset) // force the shader class to be linked as it is only used in reflection


//---------------------------------
// EnvironmentMapAsset::LoadFromMemory
//
// Loads an equirectangular texture, converts it to a cubemap, and prefilters irradiance and radiance cubemaps for IBL
//
bool EnvironmentMapAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	//load equirectangular texture
	//****************************
	std::string extension = FileUtil::ExtractExtension(GetName());
	ET_ASSERT(extension == "hdr", "Expected HDR file format!");

	stbi_set_flip_vertically_on_load(true);
	int32 width = 0;
	int32 height = 0;
	int32 channels = 0;
	float* hdrFloats = stbi_loadf_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &channels, 0);

	if (hdrFloats == nullptr)
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Failed to load hdr floats from data!", LogLevel::Warning);
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Image is too small to display!", LogLevel::Warning);
		stbi_image_free(hdrFloats);
		return false;
	}

	TextureData hdrTexture(width, height, GL_RGB16F, GL_RGB, GL_FLOAT);
	hdrTexture.Build((void*)hdrFloats);

	// we have our equirectangular texture on the GPU so we can clean up the load data on the CPU
	stbi_image_free(hdrFloats);
	hdrFloats = nullptr;

	TextureParameters params(false);
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
	hdrTexture.SetParameters(params);

	TextureData* const envCubemap = EquirectangularToCubeMap(&hdrTexture, m_CubemapRes);

	TextureData* irradianceMap;
	TextureData* radianceMap;
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
TextureData* EquirectangularToCubeMap(TextureData const* const pEqui, int32 const resolution)
{
	//Create framebuffer
	uint32 captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//Preallocate memory for cubemap
	TextureData* const envCubeMap = new TextureData(E_TextureType::CubeMap, resolution, resolution);
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
	AssetPtr<ShaderData> equiCubeShader = ResourceManager::GetInstance()->GetAssetData<ShaderData>("FwdEquiCubeShader.glsl"_hash);

	// convert HDR equirectangular environment map to cubemap equivalent
	STATE->SetShader(equiCubeShader.get());
	equiCubeShader->Upload("equirectangularMap"_hash, 0);
	STATE->LazyBindTexture(0, GL_TEXTURE_2D, pEqui->GetHandle());
	equiCubeShader->Upload("projection"_hash, CubeCaptureProjection());

	//render the cube
	//***************

	STATE->SetViewport(ivec2(0), ivec2(resolution));
	STATE->BindFramebuffer(captureFBO);
	for (uint32 i = 0; i < 6; ++i)
	{
		equiCubeShader->Upload("view"_hash, captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap->GetHandle(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	STATE->BindFramebuffer(0);

	params.genMipMaps = true;
	envCubeMap->SetParameters(params);

	STATE->BindTexture(GL_TEXTURE_2D, 0);
	STATE->SetViewport(ivec2(0), WINDOW.Dimensions);

	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);

	return envCubeMap;
}

//---------------------------------
// CubeCaptureProjection
//
// Projection matrix that can see exactly one cube face if placed at the centre of the cube
//
mat4 CubeCaptureProjection()
{
	return etm::perspective(etm::radians(90.0f), 1.0f, 0.1f, 10.0f);
}

//---------------------------------
// CubeCaptureViews
//
// Camera transformations facing the faces of a cube map from the inside
//
std::vector<mat4> CubeCaptureViews()
{
	std::vector<mat4> ret;
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));
	return ret;
}
