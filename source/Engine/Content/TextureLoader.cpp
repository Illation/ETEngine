#include "stdafx.hpp"
#include "TextureLoader.hpp"

#include <FreeImage.h>

TextureLoader::TextureLoader()
{
}

TextureLoader::~TextureLoader()
{
}

TextureData* TextureLoader::LoadContent(const std::string& assetFile)
{
	using namespace std;
	cout << "Loading Texture: " << assetFile << " . . . ";

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
		FIBITMAP *pImage = FreeImage_ConvertToType(dib, FIT_RGBF);

		uint32 width = FreeImage_GetWidth(pImage);
		uint32 height = FreeImage_GetHeight(pImage);
		BYTE* bits = FreeImage_GetBits(pImage);
		if ((bits == 0) || (width == 0) || (height == 0))
		{
			cout << "  . . . FAILED! " << endl;
			return nullptr;
		}

		GLuint texture;
		glGenTextures(1, &texture);
		STATE->BindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, m_UseSrgb?GL_SRGB:GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, (void*)bits);

		FreeImage_Unload(dib);
		FreeImage_Unload(pImage);

		// #todo this should probably happen somewhere else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		cout << "  . . . SUCCESS!" << endl;
		return new TextureData(texture, width, height);
	}
	cout << "  . . . FAILED! " << endl;
	return nullptr;
}

void TextureLoader::Destroy(TextureData* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}