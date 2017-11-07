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

		TextureData* ret = new TextureData( width, height, m_UseSrgb ? GL_SRGB : GL_RGB, GL_RGB, GL_FLOAT );
		ret->Build( (void*)bits );

		FreeImage_Unload(dib);
		FreeImage_Unload(pImage);

		TextureParameters params( true );
		ret->SetParameters( params );

		cout << "  . . . SUCCESS!" << endl;

		return ret;
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