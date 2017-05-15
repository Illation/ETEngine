#include "stdafx.hpp"
#include "SpriteFont.h"

#include "../Helper/BinaryReader.h"
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
		ZeroMemory((void*)&m_CharTable[i], sizeof(FontMetric));
		m_CharTable[i].IsValid = false;
	}
}


SpriteFont::~SpriteFont()
{
}

bool SpriteFont::IsCharValid(const wchar_t& character)
{
	if (character >= MIN_CHAR_ID && character <= MAX_CHAR_ID)
		return true;

	return false;
}