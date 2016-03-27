#include "stdafx.hpp"
#include "TextureLoader.hpp"

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

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

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	ILuint imgName;
	ilGenImages(1, &imgName);
	ilBindImage(imgName);
	if (ilLoadImage(assetFile.c_str()))
	{
		ilConvertImage(IL_RGB, IL_FLOAT);

		int width = ilGetInteger(IL_IMAGE_WIDTH);
		int height = ilGetInteger(IL_IMAGE_HEIGHT);
		ILubyte *pixelData = ilGetData();

		glTexImage2D(GL_TEXTURE_2D, 0, m_UseSrgb?GL_SRGB:GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, pixelData);

		ilBindImage(0);
		ilDeleteImage(imgName);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		cout << "  . . . SUCCESS!" << endl;
		return new TextureData(texture, width, height);
	}
	else
	{
		ILenum error = ilGetError();
		cout << "  . . . FAILED! DevIL error: " << endl << error << " - " << iluErrorString(error) << endl;
		ilBindImage(0);
		ilDeleteImage(imgName);
		return nullptr;
	}

}

void TextureLoader::Destroy(TextureData* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}