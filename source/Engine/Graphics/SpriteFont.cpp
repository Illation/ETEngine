#include "stdafx.hpp"
#include "SpriteFont.hpp"

#include "TextureData.hpp"

SpriteFont::SpriteFont() :
	m_pTexture(nullptr),
	m_CharacterCount(0),
	m_CharacterSpacing(1),
	m_FontSize(0),
	m_FontName(""),
	m_TextureWidth(0),
	m_TextureHeight(0),
	m_TextCache(std::vector<TextCache>()),
	m_BufferStart(0),
	m_BufferSize(0),
	m_IsAddedToRenderer(false)
{
	for (int32 i = 0; i < CHAR_COUNT; ++i)
	{
		m_CharTable[i].Kerning = std::map<wchar_t, vec2>();
	}
}


SpriteFont::~SpriteFont()
{
	//Cached fonts have their textures loaded through the content manager, so don't need to manage their memory
	if (!m_IsCachedFont)
	{
		SafeDelete(m_pTexture);
	}
}

bool SpriteFont::IsCharValid(const wchar_t& character)
{
	if (character >= MIN_CHAR_ID && character <= MAX_CHAR_ID)
		return true;

	return false;
}

vec2 FontMetric::GetKerningVec(wchar_t previous)
{
	vec2 kerningVec = vec2(0);
	auto kerningIt = Kerning.find(previous);
	if (kerningIt != Kerning.end())
	{
		kerningVec = kerningIt->second;
	}
	return kerningVec;
}
