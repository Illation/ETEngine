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
	cout << "Loading Cube map: " << assetFile << " . . . " << endl;
	//Get the filenames
	size_t dotPos = assetFile.find_last_of('.');
	string filestringBegin = assetFile.substr(0, dotPos);
	string filestringEnd = assetFile.substr(dotPos);
	vector<string> textureFaces;
	textureFaces.push_back(filestringBegin + "PX" + filestringEnd);
	textureFaces.push_back(filestringBegin + "NX" + filestringEnd);
	textureFaces.push_back(filestringBegin + "PY" + filestringEnd);
	textureFaces.push_back(filestringBegin + "NY" + filestringEnd);
	textureFaces.push_back(filestringBegin + "PZ" + filestringEnd);
	textureFaces.push_back(filestringBegin + "NZ" + filestringEnd);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	unsigned int width, height;
	for (GLuint i = 0; i < textureFaces.size(); i++)
	{
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
			FreeImage_FlipVertical(dib);
			FIBITMAP *pImage = FreeImage_ConvertTo24Bits(dib);

			width = FreeImage_GetWidth(pImage);
			height = FreeImage_GetHeight(pImage);
			BYTE* bits = FreeImage_GetBits(pImage);
			if ((bits == 0) || (width == 0) || (height == 0))
			{
				cout << "  . . . FAILED! " << endl;
				return nullptr;
			}

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, m_UseSrgb ? GL_SRGB : GL_RGB, width, height,
				0, GL_RGB, GL_UNSIGNED_BYTE, bits);

			FreeImage_Unload(dib);
			FreeImage_Unload(pImage);
		}
		else
		{
			cout << "  . . . FAILED! " << endl;
			return nullptr;
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	int mipNum = 1 + (int)floor(log10((float)max(width, height)) / log10(2.0));

	cout << "  . . . SUCCESS!" << endl;
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