#pragma once
#include <EtCore/Helper/Singleton.h>


class PbrPrefilter : public Singleton<PbrPrefilter>
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
protected:
private:
	friend class Singleton<PbrPrefilter>;

	PbrPrefilter();
	virtual ~PbrPrefilter();

	TextureData* m_LUT = nullptr;
};