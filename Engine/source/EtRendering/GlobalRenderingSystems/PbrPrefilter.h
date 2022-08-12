#pragma once
#include <EtRHI/GraphicsTypes/TextureData.h>


namespace et {
namespace render {


//--------------
// PbrPrefilter
//
// Handles generating lookup textures for PBR and IBL
//
class PbrPrefilter final
{
	// definitions
	//-------------
	friend class RenderingSystems;

	// construct destruct
	//--------------------
	PbrPrefilter() = default;

	// static functionality
	//----------------------
public:
	static void PopulateCubeTextureParams(rhi::TextureParameters& params);
	static void PrefilterCube(rhi::TextureData const* const source,
		rhi::TextureData*& irradiance,
		rhi::TextureData*& radiance,
		int32 const resolution,
		int32 const irradianceRes,
		int32 const radianceRes);

	// functionality
	//---------------
	void Precompute(int32 resolution);

	// accessors
	//-----------
	rhi::TextureData const* GetLUT();


	// Data
	///////

private:
	UniquePtr<rhi::TextureData> m_LUT;
};


} // namespace render
} // namespace et
