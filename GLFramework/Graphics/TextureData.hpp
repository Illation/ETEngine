#pragma once
#include "../staticDependancies/glad/glad.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

//Todo: Implement texture types with inheritance

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

class CubeMap
{
public:
	CubeMap(GLuint handle, int width,
		int height, int numMipMaps = 0) :
		m_Handle(handle),m_Width(width),
		m_Height(height),m_NumMipMaps(numMipMaps){}
	~CubeMap(){ glDeleteTextures(1, &m_Handle); }

	GLuint GetHandle() { return m_Handle; }
	int GetNumMipMaps() { return m_NumMipMaps; }

private:
	GLuint m_Handle;
	int m_Width;
	int m_Height;
	int m_NumMipMaps = 0;
};
