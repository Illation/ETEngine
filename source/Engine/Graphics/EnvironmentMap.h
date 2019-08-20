#pragma once
#include "TextureData.h"

TextureData* EquirectangularToCubeMap(TextureData const* const pEqui, int32 const resolution);
mat4 CubeCaptureProjection();
std::vector<mat4> CubeCaptureViews();

class HDRMap final
{
public:
	HDRMap(TextureData* map, TextureData* irradiance, TextureData* radiance, int32 width, int32 height, int32 numMipMaps);
	~HDRMap();

	GLuint GetHandle() { return m_Map->GetHandle(); }
	GLuint GetIrradianceHandle() { return m_Irradiance->GetHandle(); }
	GLuint GetRadianceHandle() { return m_Radiance->GetHandle(); }

	int32 GetNumMipMaps() { return m_NumMipMaps; }

private:
	TextureData* m_Map = nullptr;
	TextureData* m_Irradiance = nullptr;
	TextureData* m_Radiance = nullptr;
	int32 m_Width;
	int32 m_Height;
	int32 m_NumMipMaps = 0;
};

