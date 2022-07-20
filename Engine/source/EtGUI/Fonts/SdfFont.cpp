#include "stdafx.h"
#include "SdfFont.h"

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


//------------------------------------
// SdfFont::Metric::GetKerningVec
//
// Get the distance offset based off the previously drawn character
//
vec2 SdfFont::Metric::GetKerningVec(char32 const previous) const
{
	auto const kerningIt = m_Kerning.find(previous);

	if (kerningIt != m_Kerning.end())
	{
		return kerningIt->second;
	}

	return vec2(0.f);
}


//==========
// SDF Font
//==========


//---------------------------------
// SdfFont::c-tor
//
SdfFont::SdfFont(SdfFont const& other)
{
	*this = other;
}

//---------------------------------
// SdfFont:: copy assign op
//
// deep copy due to unique ptr
//
SdfFont& SdfFont::operator=(SdfFont const& other)
{
	m_FontFamily = other.m_FontFamily;
	m_FontSize = other.m_FontSize;
	m_IsItalic = other.m_IsItalic;
	m_Weight = other.m_Weight;

	m_LineHeight = other.m_LineHeight;
	m_Baseline = other.m_Baseline;
	m_Underline = other.m_Underline;
	m_UnderlineThickness = other.m_UnderlineThickness;

	std::copy(std::begin(other.m_CharSets), std::end(other.m_CharSets), std::begin(m_CharSets));
	m_UseKerning = other.m_UseKerning;

	if (other.m_Texture != nullptr)
	{
		m_Texture = Create<render::TextureData>(*other.m_Texture);
	}

	m_TextureAsset = other.m_TextureAsset;

	m_SdfSize = other.m_SdfSize;

	return *this;
}

//---------------------------------
// SdfFont::GetValidMetric
//
// Access the metrics for a particular character, if it is valid
//
SdfFont::Metric const* const SdfFont::GetValidMetric(char32 const character) const
{
	for (Charset const& set : m_CharSets)
	{
		if ((character >= set.m_Start) && (character <= set.m_End))
		{
			Metric const& metric = set.m_Characters[character - set.m_Start];
			if (metric.m_IsValid)
			{
				return &metric;
			}

			return nullptr;
		}
	}

	return nullptr;
}

//---------------------------------
// SdfFont::GetMetric
//
// non const access to the metrics for a particular character
//
SdfFont::Metric* const SdfFont::GetMetric(char32 const character)
{
	for (Charset& set : m_CharSets)
	{
		if ((character >= set.m_Start) && (character <= set.m_End))
		{
			return &set.m_Characters[character - set.m_Start];
		}
	}

	return nullptr;
}


//===================
// Font Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(SdfFont, "sdf font")
	END_REGISTER_CLASS(SdfFont);

	BEGIN_REGISTER_CLASS(SdfFontAsset, "sdf font asset")
		.property("is fallback", &SdfFontAsset::m_IsFallbackFont)
	END_REGISTER_CLASS_POLYMORPHIC(SdfFontAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(SdfFontAsset) // force the shader class to be linked as it is only used in reflection


// static
float const SdfFontAsset::s_KerningAdjustment = 64.f;


//---------------------------------
// SdfFontAsset::LoadFromMemory
//
// Load texture data from binary asset content, and place it on the GPU
//
bool SdfFontAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	m_Data = LoadFnt(data);

	if (m_Data == nullptr)
	{
		ET_LOG_E(ET_CTX_GUI, "FontAsset::LoadFromMemory > Loading font failed!");
		return false;
	}

	return true;
}

//---------------------------------
// SdfFontAsset::LoadFnt
//
// Loads a Sprite font from an FNT file
//
SdfFont* SdfFontAsset::LoadFnt(std::vector<uint8> const& binaryContent)
{
	core::BinaryReader binReader;
	binReader.Open(binaryContent);
	ET_ASSERT(binReader.Exists());

	if (binReader.ReadString(3u) != "BMF")
	{
		ET_TRACE_W(ET_CTX_GUI, "Font file header invalid!");
		return nullptr;
	}

	if (binReader.Read<int8>() < 3)
	{
		ET_TRACE_W(ET_CTX_GUI, "Font version invalid!");
		return nullptr;
	}

	SdfFont* font = new SdfFont();

	auto const readBlockHeader = [&binReader](uint8 const block, int32& blockSize) -> bool
		{
			if (binReader.Read<uint8>() != block)
			{
				ET_ERROR("invalid block %u header");
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
	font->m_IsItalic = binReader.Read<uint8>() & 1u << 2; // italic flag at bit 2
	font->m_Weight = static_cast<SdfFont::E_Weight>(binReader.Read<uint16>());
	binReader.SetBufferPosition(pos + 16u);

	font->m_FontFamily = binReader.ReadNullString();

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

	font->m_LineHeight = binReader.Read<uint16>();
	font->m_Baseline = binReader.Read<uint16>();
	font->m_Underline = binReader.Read<int16>();
	font->m_UnderlineThickness = binReader.Read<float>();
	
	uint16 const texWidth = binReader.Read<uint16>();
	uint16 const texHeight = binReader.Read<uint16>();

	uint16 const pagecount = binReader.Read<uint16>();
	if (pagecount != 1u)
	{
		ET_WARNING("Only one page per font supported, this font has %u", pagecount);
	}

	binReader.MoveBufferPosition(5u); // skip some stuff
	font->m_SdfSize = binReader.Read<float>();
	font->m_ThresholdPerWeight = binReader.Read<float>();

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
	ET_ASSERT(!pn.empty(), "SdfFont(.fnt): Invalid Font Sprite [Empty]");

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

	static size_t const s_PerMetricBytes = 21u;
	size_t const numChars = static_cast<size_t>(block4Size) / s_PerMetricBytes;

	char32 lastCharId = 0; 
	for (size_t i = 0; i < numChars; i++)
	{
		size_t const posChar = binReader.GetBufferPosition();
		char32 const charId = static_cast<char32>(binReader.Read<uint32>());

		if (font->m_CharSets.empty())
		{
			font->m_CharSets.push_back(SdfFont::Charset());
			font->m_CharSets.back().m_Start = charId;
		}
		else if (charId != lastCharId + 1) 
		{
			font->m_CharSets.back().m_End = lastCharId;
			font->m_CharSets.push_back(SdfFont::Charset());
			font->m_CharSets.back().m_Start = charId;
		}

		lastCharId = charId;

		font->m_CharSets.back().m_Characters.push_back(SdfFont::Metric());
		SdfFont::Metric& metric = font->m_CharSets.back().m_Characters.back();

		bool const isValid = static_cast<bool>(binReader.Read<uint8>());
		if (!isValid)
		{
			binReader.SetBufferPosition(posChar + s_PerMetricBytes); // skip
		}
		else
		{
			metric.m_IsValid = true;
			metric.m_Character = charId;
			uint16 const xPos = binReader.Read<uint16>();
			uint16 const yPos = binReader.Read<uint16>();
			metric.m_Width = binReader.Read<uint16>();
			metric.m_Height = binReader.Read<uint16>();
			metric.m_OffsetX = binReader.Read<int16>();
			metric.m_OffsetY = binReader.Read<int16>();
			metric.m_AdvanceX = static_cast<float>(binReader.Read<int16>());

			metric.m_Page = binReader.Read<uint8>();
			switch (binReader.Read<uint8>())
			{
			case 1: metric.m_Channel = 2; break;
			case 2: metric.m_Channel = 1; break;
			case 4: metric.m_Channel = 0; break;
			case 8: metric.m_Channel = 3; break;
			default:
				metric.m_Channel = 4;
				ET_ERROR("undefined channel for character %c", charId);
				break;
			}

			metric.m_TexCoord = vec2(static_cast<float>(xPos) / static_cast<float>(texWidth), static_cast<float>(yPos) / static_cast<float>(texHeight));
			binReader.SetBufferPosition(posChar + s_PerMetricBytes);
		}
	}

	ET_ASSERT(!(font->m_CharSets.empty()));
	font->m_CharSets.back().m_End = lastCharId;

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
			char32 const first = static_cast<char32>(binReader.Read<uint32>());
			char32 const second = static_cast<char32>(binReader.Read<uint32>());
			int16 const amount = binReader.Read<int16>();

			SdfFont::Metric* const metric = font->GetMetric(first);
			if ((metric != nullptr) && metric->m_IsValid)
			{
				ET_ASSERT(metric->m_Kerning.find(second) == metric->m_Kerning.cend());
				metric->m_Kerning[second] = vec2(static_cast<float>(amount) / s_KerningAdjustment, 0.f);
			}
		}
	}

	return font;
}


} // namespace gui
} // namespace et
