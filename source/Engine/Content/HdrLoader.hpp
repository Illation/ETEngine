#pragma once
#include "../staticDependancies/glad/glad.h"
#include "ContentLoader.hpp"
#include "../Graphics/TextureData.hpp"
#include <string>

class HdrLoader : public ContentLoader<HDRMap>
{
public:
	HdrLoader();
	~HdrLoader();

	void UseSrgb(bool use) { m_UseSrgb = use; }
	void SetCubemapRes(int32 res) { m_CubemapRes = res; }

protected:
	virtual HDRMap* LoadContent(const std::string& assetFile);
	virtual void Destroy(HDRMap* objToDestroy);

	bool m_UseSrgb = false;

private:
	int32 m_CubemapRes = 1024;
	int32 m_IrradianceRes = 32;
	int32 m_RadianceRes = 1024;
	int32 m_BrdfLutRes = 512;
};

