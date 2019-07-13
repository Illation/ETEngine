#pragma once
#include <string>
#include <vector>

// #todo: Implement texture types with inheritance

struct TextureParameters
{
	TextureParameters( bool useMipMaps = false, bool depthTex = false )
	{
		if(useMipMaps)
		{
			minFilter = GL_LINEAR_MIPMAP_LINEAR;
			genMipMaps = true;
		}
		isDepthTex = depthTex;
	}

	//Parameters
	int32 minFilter = GL_LINEAR;
	int32 magFilter = GL_LINEAR;
	int32 wrapS = GL_REPEAT;
	int32 wrapT = GL_REPEAT;
	int32 wrapR = GL_REPEAT;
	vec4 borderColor = vec4(1);

	bool genMipMaps = false;

	bool isDepthTex = false;
	int32 compareMode = GL_COMPARE_REF_TO_TEXTURE;
};

class TextureData
{
public:
	TextureData(GLuint handle, int32 width, int32 height, int32 depth = 1);
	TextureData(int32 width, int32 height, int32 internalFormat, GLenum format, GLenum type, int32 depth = 1);
	~TextureData();

	GLuint GetHandle() { return m_Handle; }
	ivec2 GetResolution(){return ivec2(m_Width, m_Height);}

	void Build(void* data = NULL);
	void SetParameters( TextureParameters params );

	GLenum GetTarget() { return m_Depth == 1 ? GL_TEXTURE_2D : GL_TEXTURE_3D; }

	// returns true if regenerated 
	// if its a framebuffer texture upscaling won't work properly 
	// unless it is reatached to the framebuffer object
	bool Resize( ivec2 newSize );

private:
	GLuint m_Handle;

	int32 m_Width;
	int32 m_Height;

	int32 m_Depth; // a (default) value of 1 implies a 2D texture

	//Setup
	int32 m_InternalFormat = GL_RGB;
	GLenum m_Format = GL_RGB;
	GLenum m_Type = GL_FLOAT;

	TextureParameters m_Parameters;
};

class CubeMap
{
public:
	CubeMap(GLuint handle, int32 width,
		int32 height, int32 numMipMaps = 0) :
		m_Handle(handle),m_Width(width),
		m_Height(height),m_NumMipMaps(numMipMaps){}
	~CubeMap(){ glDeleteTextures(1, &m_Handle); }

	GLuint GetHandle() { return m_Handle; }
	int32 GetNumMipMaps() { return m_NumMipMaps; }

private:
	GLuint m_Handle;
	int32 m_Width;
	int32 m_Height;
	int32 m_NumMipMaps = 0;
};

CubeMap* EquirectangularToCubeMap(TextureData* pEqui, int32 resolution);
mat4 CubeCaptureProjection();
std::vector<mat4> CubeCaptureViews();

class HDRMap
{
public:
	HDRMap(CubeMap* map, CubeMap* irradiance, CubeMap* radiance, int32 width, int32 height, int32 numMipMaps)
		:m_Map(map)
		, m_Irradiance(irradiance)
		, m_Radiance(radiance)
		,m_Width(width)
		,m_Height(height)
		,m_NumMipMaps(numMipMaps){}
	~HDRMap()
	{ 
		delete m_Map;
		delete m_Irradiance;
		delete m_Radiance;
	}

	GLuint GetHandle() { return m_Map->GetHandle(); }
	GLuint GetIrradianceHandle() { return m_Irradiance->GetHandle(); }
	GLuint GetRadianceHandle() { return m_Radiance->GetHandle(); }

	int32 GetNumMipMaps() { return m_NumMipMaps; }

private:
	CubeMap* m_Map = nullptr;
	CubeMap* m_Irradiance;
	CubeMap* m_Radiance;
	int32 m_Width;
	int32 m_Height;
	int32 m_NumMipMaps = 0;
};