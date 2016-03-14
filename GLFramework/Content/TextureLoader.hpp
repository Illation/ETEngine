#pragma once
#include "../staticDependancies/glad/glad.h"
#include "ContentLoader.hpp"
#include "..\Graphics\TextureData.hpp"
#include <string>

class TextureLoader : public ContentLoader<TextureData>
{
public:
	TextureLoader();
	~TextureLoader();

protected:

	virtual TextureData* LoadContent(const std::string& assetFile);
	virtual void Destroy(TextureData* objToDestroy);
};

