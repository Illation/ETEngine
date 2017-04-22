#pragma once

struct TextCache
{
public:
	TextCache(const std::string& text, glm::vec2 pos, glm::vec4 col) :
		Text(text), Position(pos), Color(col) {}

	const std::string Text;
	glm::vec2 Position;
	glm::vec4 Color;

private:
	TextCache& operator=(const TextCache &tmp);
};

struct FontMetric
{
	//BMFONT
	bool IsValid;
	wchar_t Character;
	USHORT Width;
	USHORT Height;
	short OffsetX;
	short OffsetY;
	short AdvanceX;
	UCHAR Page;
	UCHAR Channel;
	glm::vec2 TexCoord;
};

class TextureData;
class TextRenderer;

class SpriteFont
{
public:
	SpriteFont();
	virtual ~SpriteFont();

	static bool IsCharValid(const wchar_t& character);

	static const int MAX_CHAR_ID = 255;
	static const int MIN_CHAR_ID = 0;
	static const int CHAR_COUNT = MAX_CHAR_ID - MIN_CHAR_ID + 1;

private:
	friend class TextRenderer;
	friend class FontLoader;

	FontMetric& GetMetric(const wchar_t& character) { return m_CharTable[character - MIN_CHAR_ID]; };

	FontMetric m_CharTable[CHAR_COUNT];
	std::vector<TextCache> m_TextCache;

	short m_FontSize;
	std::string m_FontName;
	int m_CharacterCount;
	int m_CharacterSpacing;
	int m_TextureWidth;
	int m_TextureHeight;
	int m_BufferStart, m_BufferSize;
	TextureData *m_pTexture;
	bool m_IsAddedToRenderer;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpriteFont(const SpriteFont& t);
	SpriteFont& operator=(const SpriteFont& t);
};

