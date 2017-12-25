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

	FIBITMAP *pImage(0);
	if (FreeImage_FIFSupportsReading(fif))
		pImage = FreeImage_Load(fif, assetFile.c_str());
	if (pImage)
	{
		//Get dimensions and downscale if necessary
		uint32 width = FreeImage_GetWidth(pImage);
		uint32 height = FreeImage_GetHeight(pImage);
		if ((!etm::nearEquals(GRAPHICS.TextureScaleFactor, 1.f)) && (m_ForceRes));
		{
			FIBITMAP* oldImage = pImage;
			pImage = FreeImage_Rescale(pImage, width*GRAPHICS.TextureScaleFactor, height*GRAPHICS.TextureScaleFactor);
			FreeImage_Unload(oldImage);
			width = FreeImage_GetWidth(pImage);
			height = FreeImage_GetHeight(pImage);
		}

		//Convert into opengl compatible format
		FreeImage_FlipVertical(pImage);
		FIBITMAP* oldImage = pImage;
		pImage = FreeImage_ConvertToType(pImage, FIT_RGBF);
		FreeImage_Unload(oldImage);

		//Get and validate data pointer
		uint8* bits = FreeImage_GetBits(pImage);
		if ((bits == 0) || (width == 0) || (height == 0))
		{
			cout << "  . . . FAILED! " << endl;
			return nullptr;
		}

		//Upload to GPU
		TextureData* ret = new TextureData( width, height, m_UseSrgb ? GL_SRGB : GL_RGB, GL_RGB, GL_FLOAT );
		ret->Build( (void*)bits );

		FreeImage_Unload(pImage);//Destroy CPU side data

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