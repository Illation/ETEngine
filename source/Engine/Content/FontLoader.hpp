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

private:
	SpriteFont* LoadTtf(const std::vector<uint8>& binaryContent);
	SpriteFont* LoadFnt(const std::vector<uint8>& binaryContent, const std::string& assetFile);

	uint32 m_FontSize = 32;
};

