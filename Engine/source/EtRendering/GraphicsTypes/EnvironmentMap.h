#pragma once
#include "TextureData.h"

#include <EtCore/Content/AssetPointer.h>


namespace et {
namespace render {


//---------------------------------
// EnvironmentMap
//
// Contains data for skyboxes and image based lighting
//
class EnvironmentMap final
{
	REGISTRATION_FRIEND_NS(render)

	// construct destruct
	//--------------------
	EnvironmentMap() = default;
public:
	EnvironmentMap(TextureData* map, TextureData* irradiance, TextureData* radiance);
	EnvironmentMap(AssetPtr<TextureData> map, AssetPtr<TextureData> irradiance, AssetPtr<TextureData> radiance);
	~EnvironmentMap();

	// accessors
	//-----------
	uint32 GetLocation() const { return (m_Map != nullptr) ? m_Map->GetLocation() : m_MapAsset->GetLocation(); }
	TextureData const* GetIrradiance() const { return (m_Irradiance != nullptr) ? m_Irradiance : m_IrradianceAsset.get(); }
	TextureData const* GetRadiance() const { return (m_Radiance != nullptr) ? m_Radiance : m_RadianceAsset.get(); }
	int32 GetNumMipMaps() const { return m_NumMipMaps; }

	// Data
	///////
private:
	TextureData* m_Map = nullptr;
	AssetPtr<TextureData> m_MapAsset;
	TextureData* m_Irradiance = nullptr;
	AssetPtr<TextureData> m_IrradianceAsset;
	TextureData* m_Radiance = nullptr;
	AssetPtr<TextureData> m_RadianceAsset;

	int32 m_NumMipMaps = 0;
};


//---------------------------------
// EnvironmentMapAsset
//
// Loadable Environment Map
//
class EnvironmentMapAsset final : public core::Asset<EnvironmentMap, false>
{
	RTTR_ENABLE(core::Asset<EnvironmentMap, false>)
	DECLARE_FORCED_LINKING()
public:
	static std::string const s_Header;
	
	// Construct destruct
	//---------------------
	EnvironmentMapAsset() : core::Asset<EnvironmentMap, false>() {}
	virtual ~EnvironmentMapAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;
};


// Environment mapping related utility functions
TextureData* EquirectangularToCubeMap(TextureData const* const pEqui, int32 const resolution);
mat4 CubeCaptureProjection();
std::vector<mat4> CubeCaptureViews();


} // namespace render
} // namespace et
