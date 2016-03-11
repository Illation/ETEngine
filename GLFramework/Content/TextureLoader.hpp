#pragma once
#include "../staticDependancies/glad/glad.h"
#include <string>

class TextureLoader
{
public:
	TextureLoader();
	~TextureLoader();
	GLuint LoadImageToTexture(std::string filename);
};

