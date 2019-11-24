#pragma once


class PbrPrefilter final
{
public:
	void Precompute(int32 resolution);

	static void PrefilterCube(TextureData const* const source, 
		TextureData*& irradiance, 
		TextureData*& radiance, 
		int32 const resolution, 
		int32 const irradianceRes, 
		int32 const radianceRes);

	TextureData* GetLUT();
private:
	friend class RenderingSystems;

	PbrPrefilter();
	~PbrPrefilter();

	TextureData* m_LUT = nullptr;
};