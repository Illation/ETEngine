#pragma once
#include "ContentLoader.h"

#include <string>

#include <Engine/Graphics/TextureData.h>


class CubeMapLoader : public ContentLoader<CubeMap>
{
public:
	CubeMapLoader();
	~CubeMapLoader();

	void UseSrgb(bool use) { m_UseSrgb = use; }

protected:
	virtual CubeMap* LoadContent(const std::string& assetFile);
	virtual void Destroy(CubeMap* objToDestroy);

	bool m_UseSrgb = false;
};

