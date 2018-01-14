#include "stdafx.hpp"
#include "HdrLoader.hpp"

#include <FreeImage.h>

#include "../Graphics/ShaderData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"
#include "PbrPrefilter.h"

HdrLoader::HdrLoader()
{
}

HdrLoader::~HdrLoader()
{
}

HDRMap* HdrLoader::LoadContent(const std::string& assetFile)
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//load equirectangular texture
	//****************************
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(assetFile.c_str(), 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(assetFile.c_str());
	if (fif == FIF_UNKNOWN)
		return nullptr;

	FIBITMAP *dib(0);
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, assetFile.c_str());
	if (!dib)
	{
		std::cout << "Failed to load HDR image." << std::endl;
		return nullptr;
	}
	//not flipping because free image already flips automatically --maybe skybox is wrong way around and also flipped in shaders?
	FIBITMAP *pImage = FreeImage_ConvertToType(dib, FIT_RGBF);

	uint32 width = FreeImage_GetWidth(pImage);
	uint32 height = FreeImage_GetHeight(pImage);
	BYTE* data = FreeImage_GetBits(pImage);
	if ((data == 0) || (width == 0) || (height == 0))
	{
		cout << "  . . . FAILED! " << endl;
		return nullptr;
	}

	TextureData* hdrTexture = new TextureData(width, height, GL_RGB32F, GL_RGB, GL_FLOAT);
	hdrTexture->Build((void*)data);

	FreeImage_Unload(dib);
	FreeImage_Unload(pImage);

	TextureParameters params(false);
	params.wrapS = GL_CLAMP_TO_EDGE;
	params.wrapT = GL_CLAMP_TO_EDGE;
	hdrTexture->SetParameters(params);

	CubeMap* envCubemap = EquirectangularToCubeMap(hdrTexture, m_CubemapRes);
	delete hdrTexture;
	hdrTexture = nullptr;

	CubeMap* irradianceMap;
	CubeMap* radianceMap;
	PbrPrefilter::PrefilterCube(envCubemap, irradianceMap, radianceMap, m_CubemapRes, m_IrradianceRes, m_RadianceRes);

	return new HDRMap(envCubemap, irradianceMap, radianceMap, m_CubemapRes, m_CubemapRes, irradianceMap->GetNumMipMaps());
}

void HdrLoader::Destroy(HDRMap* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}