#include "stdafx.h"
#include "EnvironmentMap.h"

#include "Shader.h"

#include <FreeImage.h>

#include <EtCore/Content/ResourceManager.h>

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
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(GetName().c_str(), 0);
	if (fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilename(GetName().c_str());
	}

	if (fif == FIF_UNKNOWN)
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Cannot extract texture format from asset name!", LogLevel::Warning);
		return false;
	}
	
	ET_ASSERT(fif == FREE_IMAGE_FORMAT::FIF_HDR, "Expected HDR file format!");

	FIBITMAP* dib = nullptr;
	if (FreeImage_FIFSupportsReading(fif))
	{
		// Create a FreeImage datastream from load data. 
		// Since FreeImage requires this to be non const, we need to copy the entire load data into a temporary non const array
		BYTE* tempData = (BYTE*)malloc(data.size());
		std::copy(data.begin(), data.end(), tempData);
		FIMEMORY* stream = FreeImage_OpenMemory(tempData, static_cast<DWORD>(data.size()));
		ET_ASSERT(stream != nullptr);

		dib = FreeImage_LoadFromMemory(fif, stream);
		FreeImage_CloseMemory(stream);
		free(tempData);
	}

	if (dib == nullptr)
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Failed to load FIBITMAP from data!", LogLevel::Warning);
		return false;
	}
	
	//not flipping because freeimage already flips automatically --maybe skybox is wrong way around and also flipped in shaders?
	FIBITMAP *pImage = FreeImage_ConvertToType(dib, FIT_RGBF);
	FreeImage_Unload(dib);

	uint32 const width = FreeImage_GetWidth(pImage);
	uint32 const height = FreeImage_GetHeight(pImage);

	uint8* bits = FreeImage_GetBits(pImage);
	if ((bits == 0) || (width == 0) || (height == 0))
	{
		LOG("EnvironmentMapAsset::LoadFromMemory > Failed to get image bits from FIBITMAP!", LogLevel::Warning);
		FreeImage_Unload(pImage);
		return false;
	}

	TextureData hdrTexture(width, height, GL_RGB32F, GL_RGB, GL_FLOAT);
	hdrTexture.Build((void*)bits);

	// we have our equirectangular texture on the GPU so we can clean up the load data on the CPU
	FreeImage_Unload(pImage);
	bits = nullptr;

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
