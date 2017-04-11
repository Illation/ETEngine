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
	glm::ivec2 GetResolution(){return glm::ivec2(m_Width, m_Height);}

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

class HDRMap
{
public:
	HDRMap(GLuint handle, GLuint irradiance, GLuint radiance, GLuint brdfLut, int width, int height, int numMipMaps) 
		:m_Handle(handle)
		,m_IrradianceHandle(irradiance)
		,m_RadianceHandle(radiance)
		,m_BrdfLutHandle(brdfLut)
		,m_Width(width)
		,m_Height(height)
		,m_NumMipMaps(numMipMaps){}
	~HDRMap()
	{ 
		glDeleteTextures(1, &m_Handle); 
		glDeleteTextures(1, &m_IrradianceHandle);
		glDeleteTextures(1, &m_RadianceHandle);
		glDeleteTextures(1, &m_BrdfLutHandle);
	}

	GLuint GetHandle() { return m_Handle; }
	GLuint GetIrradianceHandle() { return m_IrradianceHandle; }
	GLuint GetRadianceHandle() { return m_RadianceHandle; }
	GLuint GetBrdfLutHandle();

	int GetNumMipMaps() { return m_NumMipMaps; }

private:
	GLuint m_Handle;
	GLuint m_IrradianceHandle;
	GLuint m_RadianceHandle;
	GLuint m_BrdfLutHandle;
	int m_Width;
	int m_Height;
	int m_NumMipMaps = 0;
};