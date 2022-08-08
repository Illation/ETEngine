#pragma once


namespace et {
namespace render {


class PbrPrefilter final
{
public:
	void Precompute(int32 resolution);

	static void PrefilterCube(rhi::TextureData const* const source, 
		rhi::TextureData*& irradiance,
		rhi::TextureData*& radiance,
		int32 const resolution, 
		int32 const irradianceRes, 
		int32 const radianceRes);

	static void PopulateCubeTextureParams(rhi::TextureParameters& params);

	rhi::TextureData* GetLUT();
private:
	friend class RenderingSystems;

	PbrPrefilter();
	~PbrPrefilter();

	rhi::TextureData* m_LUT = nullptr;
};


} // namespace render
} // namespace et
