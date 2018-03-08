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

	void SetFontSize(uint32 size) { m_FontSize = size; }
	void SetPadding(uint32 value) { m_Padding = value; }

protected:
	virtual SpriteFont* LoadContent(const std::string& assetFile);
	virtual void Destroy(SpriteFont* objToDestroy);

private:
	SpriteFont* LoadTtf(const std::vector<uint8>& binaryContent);
	SpriteFont* LoadFnt(const std::vector<uint8>& binaryContent, const std::string& assetFile);

	uint32 m_FontSize = 42;
	uint32 m_Padding = 1;
};

