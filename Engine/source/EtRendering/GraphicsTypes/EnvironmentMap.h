#pragma once
#include "TextureData.h"


//---------------------------------
// EnvironmentMap
//
// Contains data for skyboxes and image based lighting
//
class EnvironmentMap final
{
public:
	// construct destruct
	//--------------------
	EnvironmentMap(TextureData* map, TextureData* irradiance, TextureData* radiance);
	~EnvironmentMap();

	// accessors
	//-----------
	uint32 GetHandle() const { return m_Map->GetLocation(); }
	uint32 GetIrradianceHandle() const { return m_Irradiance->GetLocation(); }
	uint32 GetRadianceHandle() const { return m_Radiance->GetLocation(); }
	int32 GetNumMipMaps() const { return m_NumMipMaps; }

	// Data
	///////
private:
	TextureData* m_Map = nullptr;
	TextureData* m_Irradiance = nullptr;
	TextureData* m_Radiance = nullptr;

	int32 m_NumMipMaps = 0;
};


//---------------------------------
// EnvironmentMapAsset
//
// Loadable Environment Map
//
class EnvironmentMapAsset final : public Asset<EnvironmentMap, false>
{
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	EnvironmentMapAsset() : Asset<EnvironmentMap, false>() {}
	virtual ~EnvironmentMapAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Data
	///////
public:
	int32 m_CubemapRes = 1024;
	int32 m_IrradianceRes = 32;
	int32 m_RadianceRes = 1024;

	RTTR_ENABLE(Asset<EnvironmentMap, false>)
};


// Environment mapping related utility functions
TextureData* EquirectangularToCubeMap(TextureData const* const pEqui, int32 const resolution);
mat4 CubeCaptureProjection();
std::vector<mat4> CubeCaptureViews();
