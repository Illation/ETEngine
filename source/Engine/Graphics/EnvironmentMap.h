#pragma once

class CubeMap
{
public:
	CubeMap(GLuint handle, int32 width,
		int32 height, int32 numMipMaps = 0) :
		m_Handle(handle),m_Width(width),
		m_Height(height),m_NumMipMaps(numMipMaps){}
	~CubeMap(){ glDeleteTextures(1, &m_Handle); }

	GLuint GetHandle() const { return m_Handle; }
	int32 GetNumMipMaps() const { return m_NumMipMaps; }

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

