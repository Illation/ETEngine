#include "TextureLoader.hpp"

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <iostream>
#include <fstream>



TextureLoader::TextureLoader()
{
}


TextureLoader::~TextureLoader()
{
}


GLuint TextureLoader::LoadImageToTexture(std::string filename)
{
	using namespace std;
	cout << "Loading Texture: " << filename << " . . . ";

	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	ILuint imgName;;
	ilGenImages(1, &imgName);
	ilBindImage(imgName);
	if (ilLoadImage(filename.c_str()))
	{
		ilConvertImage(IL_RGB, IL_FLOAT);

		int width = ilGetInteger(IL_IMAGE_WIDTH);
		int height = ilGetInteger(IL_IMAGE_HEIGHT);
		ILubyte *pixelData = ilGetData();

		//glTextureStorage2D(texture, 1, GL_RGBA, width, height);
		//glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, pixelData);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, pixelData);

		ilBindImage(0);
		ilDeleteImage(imgName);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		cout << "  . . . SUCCESS!" << endl;
		return texture;
	}
	else
	{
		ILenum error = ilGetError();
		cout << "  . . . FAILED! DevIL error: " << endl << error << " - " << iluErrorString(error) << endl;
		ilBindImage(0);
		ilDeleteImage(imgName);
		return -1;
	}

}