#pragma once
#include "../staticDependancies/glad/glad.h"
#include "ContentLoader.hpp"
#include "../Graphics/SpriteFont.hpp"
#include <string>

class FontLoader : public ContentLoader<SpriteFont>
{
public:
	FontLoader();
	~FontLoader();


protected:
	virtual SpriteFont* LoadContent(const std::string& assetFile);
	virtual void Destroy(SpriteFont* objToDestroy);
};

