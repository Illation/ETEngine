#pragma once
#include "TextureData.h"

#include <EtCore/Content/AssetPointer.h>


class TextRenderer;


//---------------------------------
// TextCache
//
// Cached text that will be rendered by the TextRenderer
//
struct TextCache
{
	// construct 
	//-----------
public:
	TextCache(std::string const& text, vec2 const pos, vec4 const& col, int16 const size);

private:
	TextCache& operator=(const TextCache &tmp); // disable assignment

	// Data
	///////

public:
	std::string const Text;
	vec2 Position;
	vec4 Color;
	int16 Size;
};

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

public:
	static bool IsCharValid(const wchar_t& character);

	static const int32 MAX_CHAR_ID = 255;
	static const int32 MIN_CHAR_ID = 0;
	static const int32 CHAR_COUNT = MAX_CHAR_ID - MIN_CHAR_ID + 1;

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
	FontMetric m_CharTable[CHAR_COUNT];
	int32 m_CharacterCount = 0;
	int32 m_CharacterSpacing = 1;
	bool m_UseKerning = false;

	// sprite info
	TextureData const* m_pTexture = nullptr;
	AssetPtr<TextureData> m_TextureAsset;

	// used for rendering #todo: move this into TextRenderer
	int32 m_BufferStart = 0;
	int32 m_BufferSize = 0;
	std::vector<TextCache> m_TextCache;
	bool m_IsAddedToRenderer = false;
};

