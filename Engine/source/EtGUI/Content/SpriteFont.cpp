#include "stdafx.h"
#include "SpriteFont.h"

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/BinaryReader.h>
#include <EtCore/Reflection/Registration.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>


namespace et {
namespace gui {


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
// SpriteFont::c-tor
//
SpriteFont::SpriteFont(SpriteFont const& other)
{
	*this = other;
}

//---------------------------------
// SpriteFont::perator=
//
// deep copy due to unique ptr
//
SpriteFont& SpriteFont::operator=(SpriteFont const& other)
{
	m_FontName = other.m_FontName;
	m_FontSize = other.m_FontSize;

	std::copy(std::begin(other.m_CharTable), std::end(other.m_CharTable), std::begin(m_CharTable));
	m_CharacterCount = other.m_CharacterCount;
	m_CharacterSpacing = other.m_CharacterSpacing;
	m_UseKerning = other.m_UseKerning;

	if (other.m_Texture != nullptr)
	{
		m_Texture = Create<render::TextureData>(*other.m_Texture);
	}

	m_TextureAsset = other.m_TextureAsset;

	return *this;
}

//---------------------------------
// SpriteFont::IsCharValid
//
// Checks if a character is in the range of drawable characters
//
bool SpriteFont::IsCharValid(const wchar_t& character)
{
	return ((character >= s_MinCharId) && (character <= s_MaxCharId));
}

//---------------------------------
// SpriteFont::GetMetric
//
// Access the metrics for a particular character
//
FontMetric const& SpriteFont::GetMetric(wchar_t const& character) const
{
	return m_CharTable[character - s_MinCharId];
}

//---------------------------------
// SpriteFont::GetMetric
//
// non const access to the metrics for a particular character
//
FontMetric& SpriteFont::GetMetric(wchar_t const& character)
{
	return m_CharTable[character - s_MinCharId];
}

//---------------------------------
// SpriteFont::SetMetric
//
void SpriteFont::SetMetric(FontMetric const& metric, wchar_t const& character)
{
	m_CharTable[character - s_MinCharId] = metric;
}

//===================
// Font Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(SpriteFont, "sprite font")
	END_REGISTER_CLASS(SpriteFont);

	BEGIN_REGISTER_CLASS(FontAsset, "font asset")
	END_REGISTER_CLASS_POLYMORPHIC(FontAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(FontAsset) // force the shader class to be linked as it is only used in reflection


// static
float const FontAsset::s_KerningAdjustment = 64.f;


//---------------------------------
// TextureAsset::LoadFromMemory
//
// Load texture data from binary asset content, and place it on the GPU
//
bool FontAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	m_Data = LoadFnt(data);

	if (m_Data == nullptr)
	{
		LOG("FontAsset::LoadFromMemory > Loading font failed!", core::LogLevel::Warning);
		return false;
	}

	return true;
}

//---------------------------------
// TextureAsset::LoadFnt
//
// Loads a Sprite font from an FNT file
//
SpriteFont* FontAsset::LoadFnt(std::vector<uint8> const& binaryContent)
{
	core::BinaryReader binReader;
	binReader.Open(binaryContent);
	ET_ASSERT(binReader.Exists());

	if (binReader.ReadString(3u) != "BMF")
	{
		LOG("Font file header invalid!", core::LogLevel::Warning);
		return nullptr;
	}

	if (binReader.Read<int8>() < 3)
	{
		LOG("Font version invalid!", core::LogLevel::Warning);
		return nullptr;
	}

	SpriteFont* font = new SpriteFont();

	auto const readBlockHeader = [&binReader](uint8 const block, int32& blockSize) -> bool
		{
			if (binReader.Read<uint8>() != block)
			{
				ET_ASSERT(false, "invalid block %u header");
				return false;
			}

			blockSize = binReader.Read<int32>();
			return true;
		};

	//**********
	// BLOCK 1 *
	//**********
	int32 block1Size;
	if (!readBlockHeader(1u, block1Size))
	{
		return nullptr;
	}

	size_t pos = binReader.GetBufferPosition();

	font->m_FontSize = binReader.Read<int16>();
	binReader.SetBufferPosition(pos + 14u);

	font->m_FontName = binReader.ReadNullString();

	binReader.SetBufferPosition(pos + static_cast<size_t>(block1Size));

	//**********
	// BLOCK 2 *
	//**********
	int32 block2Size;
	if (!readBlockHeader(2u, block2Size))
	{
		return nullptr;
	}

	pos = binReader.GetBufferPosition();

	binReader.SetBufferPosition(pos + 4u);
	uint16 const texWidth = binReader.Read<uint16>();
	uint16 const texHeight = binReader.Read<uint16>();

	uint16 const pagecount = binReader.Read<uint16>();
	if (pagecount != 1u)
	{
		ET_ASSERT(false, "Only one page per font supported, this font has %u", pagecount);
	}

	binReader.SetBufferPosition(pos + static_cast<size_t>(block2Size));

	//**********
	// BLOCK 3 *
	//**********
	int32 block3Size;
	if (!readBlockHeader(3u, block3Size))
	{
		return nullptr;
	}

	pos = binReader.GetBufferPosition();

	std::string const pn = binReader.ReadNullString();
	ET_ASSERT(!pn.empty(), "SpriteFont(.fnt): Invalid Font Sprite [Empty]");

	font->m_TextureAsset = core::ResourceManager::Instance()->GetAssetData<render::TextureData>(core::HashString(pn.c_str()));
	ET_ASSERT(font->m_TextureAsset->GetResolution() == ivec2(static_cast<int32>(texWidth), static_cast<int32>(texHeight)));

	binReader.SetBufferPosition(pos + static_cast<size_t>(block3Size));

	//**********
	// BLOCK 4 *
	//**********
	int32 block4Size;
	if (!readBlockHeader(4u, block4Size))
	{
		return nullptr;
	}

	pos = binReader.GetBufferPosition();

	auto numChars = block4Size / 20;
	font->m_CharacterCount = numChars;

	for (int32 i = 0; i < numChars; i++)
	{
		size_t const posChar = binReader.GetBufferPosition();
		wchar_t const charId = (wchar_t)(binReader.Read<uint32>());

		if (!(font->IsCharValid(charId)))
		{
			LOG("SpriteFont::Load > SpriteFont(.fnt): Invalid Character", core::LogLevel::Warning);
			binReader.SetBufferPosition(posChar + 20u);
		}
		else
		{
			auto metric = &(font->GetMetric(charId));
			metric->IsValid = true;
			metric->Character = charId;
			auto xPos = binReader.Read<uint16>();
			auto yPos = binReader.Read<uint16>();
			metric->Width = binReader.Read<uint16>();
			metric->Height = binReader.Read<uint16>();
			metric->OffsetX = binReader.Read<int16>();
			metric->OffsetY = binReader.Read<int16>();
			metric->AdvanceX = binReader.Read<int16>();

			metric->Page = binReader.Read<uint8>();
			switch (binReader.Read<uint8>())
			{
			case 1: metric->Channel = 2; break;
			case 2: metric->Channel = 1; break;
			case 4: metric->Channel = 0; break;
			case 8: metric->Channel = 3; break;
			default:
				metric->Channel = 4;
				ET_ASSERT(false, "undefined channel for character %c", charId);
				break;
			}

			metric->TexCoord = vec2(static_cast<float>(xPos) / static_cast<float>(texWidth), static_cast<float>(yPos) / static_cast<float>(texHeight));
			binReader.SetBufferPosition(posChar + 20u);
		}
	}

	binReader.SetBufferPosition(pos + static_cast<size_t>(block4Size));

	//**********
	// BLOCK 5 *
	//**********
	if (binReader.GetBufferPosition() < binReader.GetBufferSize())
	{
		int32 block5Size;
		if (!readBlockHeader(5u, block5Size))
		{
			return nullptr;
		}

		int32 const numKerningPairs = block5Size / 10;
		
		font->m_UseKerning = true;

		for (int32 i = 0; i < numKerningPairs; i++)
		{
			wchar_t const first = static_cast<wchar_t>(binReader.Read<uint32>());
			wchar_t const second = static_cast<wchar_t>(binReader.Read<uint32>());
			int16 const amount = binReader.Read<int16>();

			FontMetric& metric = font->GetMetric(first);
			if (metric.IsValid)
			{
				ET_ASSERT(metric.Kerning.find(second) == metric.Kerning.cend());
				metric.Kerning[second] = vec2(static_cast<float>(amount) / s_KerningAdjustment, 0.f);
			}
		}
	}

	return font;
}


} // namespace gui
} // namespace et
