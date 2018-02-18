#include "stdafx.hpp"
#include "CubeMapLoader.hpp"

#include <FreeImage.h>

CubeMapLoader::CubeMapLoader()
{
}

CubeMapLoader::~CubeMapLoader()
{
}

CubeMap* CubeMapLoader::LoadContent(const std::string& assetFile)
{
	ivec2 logPos = Logger::GetCursorPosition();
	std::string loadingString = std::string("Loading Cube map: ") + assetFile + " . . .";

	LOG(loadingString + " . . . generating texture          ", Info, false, logPos);
	//Get the filenames
	size_t dotPos = assetFile.find_last_of('.');
	std::string filestringBegin = assetFile.substr(0, dotPos);
	std::string filestringEnd = assetFile.substr(dotPos);
	std::vector<std::string> textureFaces;
	textureFaces.push_back(filestringBegin + "PX" + filestringEnd);
	textureFaces.push_back(filestringBegin + "NX" + filestringEnd);
	textureFaces.push_back(filestringBegin + "PY" + filestringEnd);
	textureFaces.push_back(filestringBegin + "NY" + filestringEnd);
	textureFaces.push_back(filestringBegin + "PZ" + filestringEnd);
	textureFaces.push_back(filestringBegin + "NZ" + filestringEnd);

	GLuint texture;
	glGenTextures(1, &texture);
	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, texture);

	uint32 width = 0, height = 0;
	int32 mipNum = 0;
	for (GLuint i = 0; i < textureFaces.size(); i++)
	{
		LOG(loadingString + " . . . loading file          ", Info, false, logPos);
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(assetFile.c_str(), 0);
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(assetFile.c_str());
		if (fif == FIF_UNKNOWN)
			return nullptr;

		FIBITMAP *dib(0);
		if (FreeImage_FIFSupportsReading(fif))
			dib = FreeImage_Load(fif, assetFile.c_str());
		if (dib)
		{
			LOG(loadingString + " . . . converting format          ", Info, false, logPos);
			FreeImage_FlipVertical(dib);
			FIBITMAP *pImage = FreeImage_ConvertTo24Bits(dib);

			width = FreeImage_GetWidth(pImage);
			height = FreeImage_GetHeight(pImage);
			BYTE* bits = FreeImage_GetBits(pImage);
			if ((bits == 0) || (width == 0) || (height == 0))
			{
				LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
				return nullptr;
			}

			LOG(loadingString + " . . . uploading to GPU          ", Info, false, logPos);
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, m_UseSrgb ? GL_SRGB : GL_RGB, width, height,
				0, GL_RGB, GL_UNSIGNED_BYTE, bits);

			FreeImage_Unload(dib);
			FreeImage_Unload(pImage);
		}
		else
		{
			LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
			return nullptr;
		}
	}

	mipNum = 1 + (int32)floor(log10((float)std::max(width, height)) / log10(2.0));

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	LOG(loadingString + " . . . SUCCESS!          ", Info, false, logPos);
	return new CubeMap(texture, width, height, mipNum);
}

void CubeMapLoader::Destroy(CubeMap* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}