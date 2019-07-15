#pragma once
#include <EtCore/Helper/Singleton.h>


class PbrPrefilter : public Singleton<PbrPrefilter>
{
public:
	void Precompute(int32 resolution);

	static void PrefilterCube(CubeMap* source, CubeMap* &irradiance, CubeMap* &radiance, int32 resolution, int32 irradianceRes, int32 radianceRes);

	TextureData* GetLUT();
protected:
private:
	friend class Singleton<PbrPrefilter>;

	PbrPrefilter();
	virtual ~PbrPrefilter();

	TextureData* m_LUT = nullptr;
};