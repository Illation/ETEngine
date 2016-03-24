#include "stdafx.hpp"
#include "CubeMapLoader.hpp"

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

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

	int width, height;
	for (GLuint i = 0; i < textureFaces.size(); i++)
	{
		ILuint imgName;
		ilGenImages(1, &imgName);
		ilBindImage(imgName);
		if (ilLoadImage(textureFaces[i].c_str()))
		{
			ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);

			width = ilGetInteger(IL_IMAGE_WIDTH);
			height = ilGetInteger(IL_IMAGE_HEIGHT);
			ILubyte *pixelData = ilGetData();

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, m_UseSrgb ? GL_SRGB : GL_RGB, width, height,
				0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);

			ilBindImage(0);
			ilDeleteImage(imgName);

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

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	cout << "  . . . SUCCESS!" << endl;
	return new CubeMap(texture, width, height);
}

void CubeMapLoader::Destroy(CubeMap* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}