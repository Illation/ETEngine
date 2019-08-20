#include "stdafx.h"
#include "SpriteFont.h"


//============
// Text Cache
//============


//---------------------------------
// TextCache::c-tor
//
// Initialize a text cache
//
TextCache::TextCache(std::string const& text, vec2 const pos, vec4 const& col, int16 const size) 
	: Text(text)
	, Position(pos)
	, Color(col)
	, Size(size)
{ }


//=============
// Font Metric
//=============


//---------------------------------
// FontMetric::GetKerningVec
//
// Get the distance offset based off the previously drawn character
//
vec2 FontMetric::GetKerningVec(wchar_t previous) const
{
	auto kerningIt = Kerning.find(previous);

	if (kerningIt != Kerning.end())
	{
		return kerningIt->second;
	}

	return vec2(0.f);
}


//=============
// Sprite Font
//=============


//---------------------------------
// SpriteFont::IsCharValid
//
// Checks if a character is in the range of drawable characters
//
bool SpriteFont::IsCharValid(const wchar_t& character)
{
	return ((character >= MIN_CHAR_ID) && (character <= MAX_CHAR_ID));
}

//---------------------------------
// SpriteFont::d-tor
//
// Destroy the texture data if it was not loaded from the resource managegr
//
SpriteFont::~SpriteFont()
{
	if (m_TextureAsset == nullptr)
	{
		SafeDelete(m_pTexture);
	}
}

//---------------------------------
// SpriteFont::GetMetric
//
// Access the metrics for a particular character
//
FontMetric& SpriteFont::GetMetric(wchar_t const& character)
{
	return m_CharTable[character - MIN_CHAR_ID];
}

//---------------------------------
// SpriteFont::SetMetric
//
void SpriteFont::SetMetric(FontMetric const& metric, wchar_t const& character)
{
	m_CharTable[character - MIN_CHAR_ID] = metric;
}
