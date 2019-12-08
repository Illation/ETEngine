#include "stdafx.h"
#include "SpriteFont.h"

#include "TextureData.h"
#include "Shader.h"

#include <ft2build.h>
#include <freetype/freetype.h>

#include <EtCore/FileSystem/BinaryReader.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


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
	return ((character >= s_MinCharId) && (character <= s_MaxCharId));
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
	using namespace rttr;

	registration::class_<FontAsset>("font asset")
		.constructor<FontAsset const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("size", &FontAsset::m_FontSize)
		.property("padding", &FontAsset::m_Padding)
		.property("spread", &FontAsset::m_Spread)
		.property("highres", &FontAsset::m_HighRes);

	rttr::type::register_converter_func([](FontAsset& asset, bool& ok) -> I_Asset*
	{
		ok = true;
		return new FontAsset(asset);
	});
}
DEFINE_FORCED_LINKING(FontAsset) // force the shader class to be linked as it is only used in reflection


//---------------------------------
// TextureAsset::LoadFromMemory
//
// Load texture data from binary asset content, and place it on the GPU
//
bool FontAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	// temp
	std::vector<uint8> binaryContent = data;
	std::string extension = FileUtil::ExtractExtension(GetName());

	if (extension == "ttf")
	{
		m_Data = LoadTtf(binaryContent);
	}
	else if (extension == "fnt")
	{
		m_Data = LoadFnt(binaryContent);
	}
	else
	{
		LOG("FontAsset::LoadFromMemory > Cannot load audio data with this extension! Supported exensions: [.ttf/.fnt]", LogLevel::Warning);
		return false;
	}

	if (m_Data == nullptr)
	{
		LOG("FontAsset::LoadFromMemory > Loading font failed!", LogLevel::Warning);
		return false;
	}

	return true;
}

//---------------------------------
// TextureAsset::LoadTtf
//
// Rasterizes a ttf font into a SDF texture and generates SpriteFont data from it
//
SpriteFont* FontAsset::LoadTtf(const std::vector<uint8>& binaryContent)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		LOG("FREETYPE: Could not init FreeType Library", Warning);

	FT_Face face;
	if (FT_New_Memory_Face(ft, binaryContent.data(), (FT_Long)binaryContent.size(), 0, &face))
		LOG("FREETYPE: Failed to load font", Warning);

	FT_Set_Pixel_Sizes(face, 0, m_FontSize);

	SpriteFont* pFont = new SpriteFont();
	pFont->m_FontSize = (int16)m_FontSize;
	pFont->m_FontName = std::string(face->family_name) + " - " + face->style_name;
	pFont->m_CharacterCount = face->num_glyphs;

	pFont->m_UseKerning = FT_HAS_KERNING(face) != 0;

	//Atlasing helper variables
	ivec2 startPos[4] = { ivec2(0), ivec2(0), ivec2(0), ivec2(0) };
	ivec2 maxPos[4] = { ivec2(0), ivec2(0), ivec2(0), ivec2(0) };
	bool horizontal = false;//Direction this pass expands the map in (internal moves are !horizontal)
	uint32 posCount = 1;//internal move count in this pass
	uint32 curPos = 0;//internal move count
	uint32 channel = 0;//channel to add to

	uint32 totPadding = m_Padding + m_Spread;

	//Load individual character metrics
	std::map<int32, FontMetric*> characters;
	for (int32 c = 0; c < SpriteFont::s_CharCount - 1; c++)
	{
		FontMetric* metric = &(pFont->GetMetric(static_cast<wchar_t>(c)));
		metric->Character = static_cast<wchar_t>(c);

		uint32 glyphIdx = FT_Get_Char_Index(face, c);

		if (pFont->m_UseKerning && glyphIdx)
		{
			for (int32 previous = 0; previous < SpriteFont::s_CharCount - 1; previous++)
			{
				FT_Vector delta;

				uint32 prevIdx = FT_Get_Char_Index(face, previous);
				FT_Get_Kerning(face, prevIdx, glyphIdx, FT_KERNING_DEFAULT, &delta);

				if (delta.x || delta.y)
				{
					metric->Kerning[static_cast<wchar_t>(previous)] = vec2((float)delta.x / 64.f, (float)delta.y / 64.f);
				}
			}
		}

		if (FT_Load_Glyph(face, glyphIdx, FT_LOAD_DEFAULT))
		{
			LOG("FREETYPE: Failed to load glyph", Warning);
			continue;
		}

		uint32 width = face->glyph->bitmap.width + totPadding * 2;
		uint32 height = face->glyph->bitmap.rows + totPadding * 2;

		metric->Width = (uint16)width;
		metric->Height = (uint16)height;
		metric->OffsetX = (int16)face->glyph->bitmap_left + (int16)totPadding;
		metric->OffsetY = -(int16)(face->glyph->bitmap_top + (int16)totPadding);
		metric->AdvanceX = (float)face->glyph->advance.x / 64.f;

		//Generate atlas coordinates
		metric->Page = 0;
		metric->Channel = (uint8)channel;
		metric->TexCoord = etm::vecCast<float>(startPos[channel]);
		if (horizontal)
		{
			maxPos[channel].y = std::max(maxPos[channel].y, startPos[channel].y + (int32)height);
			startPos[channel].y += height;
			maxPos[channel].x = std::max(maxPos[channel].x, startPos[channel].x + (int32)width);
		}
		else
		{
			maxPos[channel].x = std::max(maxPos[channel].x, startPos[channel].x + (int32)width);
			startPos[channel].x += width;
			maxPos[channel].y = std::max(maxPos[channel].y, startPos[channel].y + (int32)height);
		}
		channel++;
		if (channel == 4)
		{
			channel = 0;
			curPos++;
			if (curPos == posCount)
			{
				curPos = 0;
				horizontal = !horizontal;
				if (horizontal)
				{
					for (uint8 cha = 0; cha < 4; ++cha)startPos[cha] = ivec2(maxPos[cha].x, 0);
				}
				else
				{
					for (uint8 cha = 0; cha < 4; ++cha)startPos[cha] = ivec2(0, maxPos[cha].y);
					posCount++;
				}
			}
		}

		metric->IsValid = true;

		characters[c] = metric;
	}

	int32 const texWidth = std::max(std::max(maxPos[0].x, maxPos[1].x), std::max(maxPos[2].x, maxPos[3].x));
	int32 const texHeight = std::max(std::max(maxPos[0].y, maxPos[1].y), std::max(maxPos[2].y, maxPos[3].y));

	//Setup rendering
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	TextureData* const texture = new TextureData(ivec2(texWidth, texHeight), E_ColorFormat::RGBA16f, E_ColorFormat::RGBA, E_DataType::Float);
	texture->Build();
	texture->SetParameters(params);
	pFont->m_pTexture = texture;

	T_FbLoc captureFBO;
	T_RbLoc captureRBO;

	api->GenFramebuffers(1, &captureFBO);
	api->GenRenderBuffers(1, &captureRBO);

	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);

	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24, ivec2(texWidth, texHeight));
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24, captureRBO);
	api->LinkTextureToFbo2D(0, pFont->m_pTexture->GetLocation(), 0);

	api->SetViewport(ivec2(0), ivec2(texWidth, texHeight));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

	AssetPtr<ShaderData> computeSdf = ResourceManager::Instance()->GetAssetData<ShaderData>("ComputeGlyphSDF.glsl"_hash);
	api->SetShader(computeSdf.get());
	computeSdf->Upload("uSpread"_hash, static_cast<float>(m_Spread));
	computeSdf->Upload("uHighRes"_hash, static_cast<float>(m_HighRes));

	params.wrapS = E_TextureWrapMode::ClampToBorder;
	params.wrapT = E_TextureWrapMode::ClampToBorder;
	params.borderColor = vec4(0);

	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);

	//Render to Glyphs atlas
	FT_Set_Pixel_Sizes(face, 0, m_FontSize * m_HighRes);
	api->SetPixelUnpackAlignment(1);
	for (auto& character : characters)
	{
		auto metric = character.second;

		uint32 glyphIdx = FT_Get_Char_Index(face, metric->Character);
		if (FT_Load_Glyph(face, glyphIdx, FT_LOAD_DEFAULT))
		{
			LOG("FREETYPE: Failed to load glyph", Warning);
			continue;
		}
		if (!(face->glyph->format == FT_GLYPH_FORMAT_BITMAP))
		{
			if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
			{
				LOG("FREETYPE: Failed to render glyph", Warning);
				continue;
			}
		}

		uint32 width = face->glyph->bitmap.width;
		uint32 height = face->glyph->bitmap.rows;
		auto pTexture = new TextureData(ivec2(width, height), E_ColorFormat::R8, E_ColorFormat::Red, E_DataType::UByte);
		pTexture->Build(face->glyph->bitmap.buffer); 
		pTexture->SetParameters(params);

		ivec2 res = ivec2(metric->Width - m_Padding * 2, metric->Height - m_Padding * 2);
		api->SetViewport(etm::vecCast<int32>(metric->TexCoord) + ivec2(m_Padding), res);
		computeSdf->Upload("uTex"_hash, static_cast<TextureData const*>(pTexture));
		computeSdf->Upload("uChannel"_hash, static_cast<int32>(metric->Channel));
		computeSdf->Upload("uResolution"_hash, etm::vecCast<float>(res));
		RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();

		delete pTexture;

		//modify texture coordinates after rendering sprites
		metric->TexCoord = metric->TexCoord / vec2(static_cast<float>(texWidth), static_cast<float>(texHeight));
	}
	api->SetPixelUnpackAlignment(4);

	//Cleanup
	api->SetBlendEnabled(false);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	api->BindFramebuffer(0);
	api->BindRenderbuffer(0);
	api->SetViewport(ivec2(0), Viewport::GetCurrentViewport()->GetDimensions());

	api->DeleteRenderBuffers(1, &captureRBO);
	api->DeleteFramebuffers(1, &captureFBO);

	return pFont;
}

//---------------------------------
// TextureAsset::LoadFnt
//
// Loads a Sprite font from an FNT file
//
SpriteFont* FontAsset::LoadFnt(const std::vector<uint8>& binaryContent)
{
	auto pBinReader = new BinaryReader(); //Prevent memory leaks
	pBinReader->Open(binaryContent);

	if (!pBinReader->Exists())
	{
		delete pBinReader;
		LOG("SpriteFont::Load > Failed to read the assetFile!", Warning);

		return nullptr;
	}
	bool valid = false;
	if (pBinReader->Read<char>() == 'B')
	{
		if (pBinReader->Read<char>() == 'M')
		{
			if (pBinReader->Read<char>() == 'F')
			{
				valid = true;
			}
		}
	}
	if (!valid)
	{
		LOG("Font file header invalid!", Warning);
		return nullptr;
	}
	if (pBinReader->Read<char>() < 3)
	{
		LOG("Font version invalid!", Warning);
		return nullptr;
	}

	SpriteFont* pFont = new SpriteFont();

	//**********
	// BLOCK 0 *
	//**********
	pBinReader->Read<char>();
	auto Block0Size = pBinReader->Read<int32>();
	int32 pos = pBinReader->GetBufferPosition();
	pFont->m_FontSize = pBinReader->Read<int16>();
	pBinReader->SetBufferPosition(pos + 14);
	std::string fn;
	char cur = pBinReader->Read<char>();
	while (cur != '\0')
	{
		fn += cur;
		cur = pBinReader->Read<char>();
	}
	pFont->m_FontName = fn;
	pBinReader->SetBufferPosition(pos + Block0Size);
	//**********
	// BLOCK 1 *
	//**********
	pBinReader->Read<char>();
	auto Block1Size = pBinReader->Read<int32>();
	pos = pBinReader->GetBufferPosition();
	pBinReader->SetBufferPosition(pos + 4);
	uint16 const texWidth = pBinReader->Read<uint16>();
	uint16 const texHeight = pBinReader->Read<uint16>();
	uint16 const pagecount = pBinReader->Read<uint16>();

	ET_ASSERT(pagecount == 1u, "SpriteFont(.fnt): Only one texture per font allowed");

	pBinReader->SetBufferPosition(pos + Block1Size);
	//**********
	// BLOCK 2 *
	//**********
	pBinReader->Read<char>();
	auto Block2Size = pBinReader->Read<int32>();
	pos = pBinReader->GetBufferPosition();
	std::string pn;
	cur = pBinReader->Read<char>();
	while (cur != '\0')
	{
		pn += cur;
		cur = pBinReader->Read<char>();
	}

	ET_ASSERT(!pn.empty(), "SpriteFont(.fnt): Invalid Font Sprite [Empty]");

	pFont->m_TextureAsset = ResourceManager::Instance()->GetAssetData<TextureData>(GetHash(pn));
	pFont->m_pTexture = pFont->m_TextureAsset.get();
	pBinReader->SetBufferPosition(pos + Block2Size);
	//**********
	// BLOCK 3 *
	//**********
	pBinReader->Read<char>();
	auto Block3Size = pBinReader->Read<int32>();
	pos = pBinReader->GetBufferPosition();
	auto numChars = Block3Size / 20;
	pFont->m_CharacterCount = numChars;

	for (int32 i = 0; i < numChars; i++)
	{
		int32 const posChar = pBinReader->GetBufferPosition();
		wchar_t const charId = (wchar_t)(pBinReader->Read<uint32>());

		if (!(pFont->IsCharValid(charId)))
		{
			LOG("SpriteFont::Load > SpriteFont(.fnt): Invalid Character", Warning);
			pBinReader->SetBufferPosition(posChar + 20);
		}
		else
		{
			auto metric = &(pFont->GetMetric(charId));
			metric->IsValid = true;
			metric->Character = charId;
			auto xPos = pBinReader->Read<uint16>();
			auto yPos = pBinReader->Read<uint16>();
			metric->Width = pBinReader->Read<uint16>();
			metric->Height = pBinReader->Read<uint16>();
			metric->OffsetX = pBinReader->Read<int16>();
			metric->OffsetY = pBinReader->Read<int16>();
			metric->AdvanceX = pBinReader->Read<int16>();
			metric->Page = pBinReader->Read<uint8>();
			auto chan = pBinReader->Read<uint8>();
			switch (chan)
			{
			case 1: metric->Channel = 2; break;
			case 2: metric->Channel = 1; break;
			case 4: metric->Channel = 0; break;
			case 8: metric->Channel = 3; break;
			default: metric->Channel = 4; break;
			}
			metric->TexCoord = vec2(static_cast<float>(xPos) / static_cast<float>(texWidth), static_cast<float>(yPos) / static_cast<float>(texHeight));
			pBinReader->SetBufferPosition(posChar + 20);
		}
	}

	delete pBinReader;

	return pFont;
}
