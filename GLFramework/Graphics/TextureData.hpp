#pragma once
#include "../staticDependancies/glad/glad.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
class TextureData
{
public:
	TextureData(GLuint handle, int width, int height);
	~TextureData();

	GLuint GetHandle() { return m_Handle; }

private:
	GLuint m_Handle;
	int m_Width;
	int m_Height;
};

