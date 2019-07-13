#pragma once

class TextureData;
class TextRenderer;

struct TextCache
{
public:
	TextCache(const std::string& text, vec2 pos, vec4 col, int16 size) :
		Text(text), Position(pos), Color(col), Size(size) {}

	const std::string Text;
	vec2 Position;
	vec4 Color;
	int16 Size;

private:
	TextCache& operator=(const TextCache &tmp);
};

struct FontMetric
{
	vec2 GetKerningVec(wchar_t previous);

	bool IsValid = false;
	wchar_t Character = 0;

	uint16 Width = 0;
	uint16 Height = 0;
	int16 OffsetX = 0;
	int16 OffsetY = 0;

	float AdvanceX = 0;
	std::map<wchar_t, vec2> Kerning;

	uint8 Page = 0;
	uint8 Channel = 0;
	vec2 TexCoord = 0;
};

class SpriteFont
{
public:
	SpriteFont();
	virtual ~SpriteFont();

	static bool IsCharValid(const wchar_t& character);

	static const int32 MAX_CHAR_ID = 255;
	static const int32 MIN_CHAR_ID = 0;
	static const int32 CHAR_COUNT = MAX_CHAR_ID - MIN_CHAR_ID + 1;

	TextureData* GetAtlas() { return m_pTexture; }

	int16 GetFontSize() { return m_FontSize; }

private:
	friend class TextRenderer;
	friend class FontLoader;

	FontMetric& GetMetric(const wchar_t& character) { return m_CharTable[character - MIN_CHAR_ID]; };
	void SetMetric(const FontMetric& metric, const wchar_t& character) { m_CharTable[character - MIN_CHAR_ID] = metric; }

	FontMetric m_CharTable[CHAR_COUNT];
	std::vector<TextCache> m_TextCache;

	int16 m_FontSize;
	std::string m_FontName;
	int32 m_CharacterCount;
	int32 m_CharacterSpacing;
	bool m_UseKerning = false;
	int32 m_TextureWidth;
	int32 m_TextureHeight;
	int32 m_BufferStart, m_BufferSize;
	TextureData *m_pTexture;
	bool m_IsAddedToRenderer;

	bool m_IsCachedFont = false;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpriteFont(const SpriteFont& t);
	SpriteFont& operator=(const SpriteFont& t);
};

