#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtRHI/GraphicsTypes/TextureData.h>


namespace et {
	REGISTRATION_NS(render);
}


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
	EnvironmentMap(rhi::TextureData* map, rhi::TextureData* irradiance, rhi::TextureData* radiance);
	EnvironmentMap(AssetPtr<rhi::TextureData> map, AssetPtr<rhi::TextureData> irradiance, AssetPtr<rhi::TextureData> radiance);
	~EnvironmentMap();

	// accessors
	//-----------
	uint32 GetLocation() const { return (m_Map != nullptr) ? m_Map->GetLocation() : m_MapAsset->GetLocation(); }
	rhi::TextureData const* GetIrradiance() const { return (m_Irradiance != nullptr) ? m_Irradiance : m_IrradianceAsset.get(); }
	rhi::TextureData const* GetRadiance() const { return (m_Radiance != nullptr) ? m_Radiance : m_RadianceAsset.get(); }
	int32 GetNumMipMaps() const { return m_NumMipMaps; }

	// Data
	///////
private:
	rhi::TextureData* m_Map = nullptr;
	AssetPtr<rhi::TextureData> m_MapAsset;
	rhi::TextureData* m_Irradiance = nullptr;
	AssetPtr<rhi::TextureData> m_IrradianceAsset;
	rhi::TextureData* m_Radiance = nullptr;
	AssetPtr<rhi::TextureData> m_RadianceAsset;

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
rhi::TextureData* EquirectangularToCubeMap(rhi::TextureData const* const pEqui, int32 const resolution);
mat4 CubeCaptureProjection();
std::vector<mat4> CubeCaptureViews();


} // namespace render
} // namespace et
