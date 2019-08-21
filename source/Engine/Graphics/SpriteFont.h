#pragma once
#include "TextureData.h"

#include <EtCore/Content/AssetPointer.h>


class TextRenderer;


//---------------------------------
// FontMetric
//
// Information about positioning of individual characters in a font
//
struct FontMetric
{
	// accessors
	//-----------
	vec2 GetKerningVec(wchar_t previous) const;

	// Data
	///////

	bool IsValid = false;
	wchar_t Character = 0;

	// dimensions
	uint16 Width = 0;
	uint16 Height = 0;
	int16 OffsetX = 0;
	int16 OffsetY = 0;

	// spacing between characters
	float AdvanceX = 0;
	std::map<wchar_t, vec2> Kerning;

	// addressing in texture
	uint8 Page = 0;
	uint8 Channel = 0;
	vec2 TexCoord = 0;
};

//---------------------------------
// SpriteFont
//
// Font renderable in realtime
//
class SpriteFont final
{
	// definitions
	//-------------
private:
	friend class TextRenderer;
	friend class FontLoader;
	friend class FontAsset;

public:
	static bool IsCharValid(const wchar_t& character);

	static int32 const s_MinCharId = 0;
	static int32 const s_MaxCharId = 255;
	static int32 const s_CharCount = s_MaxCharId - s_MinCharId + 1;

	// construct destruct
	//--------------------
	SpriteFont() = default;
	~SpriteFont();

private:
	SpriteFont(const SpriteFont& t);
	SpriteFont& operator=(const SpriteFont& t);

	// accessors
	//-----------
public:
	TextureData const* GetAtlas() const { return m_pTexture; }
	int16 GetFontSize() const { return m_FontSize; }
	FontMetric const& GetMetric(wchar_t const& character) const;

	// utility
	//---------
private:
	FontMetric& GetMetric(wchar_t const& character);
	void SetMetric(FontMetric const& metric, wchar_t const& character);

	// Data
	///////

	// font info
	std::string m_FontName;
	int16 m_FontSize = 0;

	// character info
	FontMetric m_CharTable[s_CharCount];
	int32 m_CharacterCount = 0;
	int32 m_CharacterSpacing = 1;
	bool m_UseKerning = false;

	// sprite info
	TextureData const* m_pTexture = nullptr;
	AssetPtr<TextureData> m_TextureAsset;
};

//---------------------------------
// FontAsset
//
// Loadable Font Data
//
class FontAsset final : public Asset<SpriteFont, false>
{
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	FontAsset() : Asset<SpriteFont, false>() {}
	virtual ~FontAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

private:
	SpriteFont* LoadTtf(const std::vector<uint8>& binaryContent);
	SpriteFont* LoadFnt(const std::vector<uint8>& binaryContent);

	// Data
	///////
public:
	uint32 m_FontSize = 42u;
	uint32 m_Padding = 1u;
	uint32 m_Spread = 5u;
	uint32 m_HighRes = 32u;

	RTTR_ENABLE(Asset<SpriteFont, false>)
};

