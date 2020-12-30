#include "stdafx.h"
#include "EditableFontAsset.h"

#include <ft2build.h>
#include <freetype/freetype.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace pl {


//=====================
// Editable Font Asset
//=====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableFontAsset, "editable font asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableFontAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableFontAsset) // force the asset class to be linked as it is only used in reflection


//-------------------------------------
// EditableFontAsset::LoadFromMemory
//
bool EditableFontAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	ET_ASSERT(core::FileUtil::ExtractExtension(m_Asset->GetName()) == "ttf");

	// Create shader data
	SetData(LoadTtf(data));
	render::SpriteFont* const fontData = GetData();

	if (fontData == nullptr)
	{
		LOG("EditableFontAsset::LoadFromMemory > Loading font failed!", core::LogLevel::Warning);
		return false;
	}

	return true;
}

//---------------------------------
// TextureAsset::LoadTtf
//
// Rasterizes a ttf font into a SDF texture and generates SpriteFont data from it
//
render::SpriteFont* EditableFontAsset::LoadTtf(const std::vector<uint8>& binaryContent)
{
	render::FontAsset* asset = static_cast<render::FontAsset*>(m_Asset);

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		LOG("FREETYPE: Could not init FreeType Library", core::LogLevel::Warning);
	}

	FT_Face face;
	if (FT_New_Memory_Face(ft, binaryContent.data(), (FT_Long)binaryContent.size(), 0, &face))
	{
		LOG("FREETYPE: Failed to load font", core::LogLevel::Warning);
	}

	FT_Set_Pixel_Sizes(face, 0, asset->m_FontSize);

	render::SpriteFont* pFont = new render::SpriteFont();
	pFont->m_FontSize = static_cast<int16>(asset->m_FontSize);
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

	uint32 totPadding = asset->m_Padding + asset->m_Spread;

	//Load individual character metrics
	std::map<int32, render::FontMetric*> characters;
	for (int32 c = 0; c < render::SpriteFont::s_CharCount - 1; c++)
	{
		render::FontMetric* metric = &(pFont->GetMetric(static_cast<wchar_t>(c)));
		metric->Character = static_cast<wchar_t>(c);

		uint32 glyphIdx = FT_Get_Char_Index(face, c);

		if (pFont->m_UseKerning && glyphIdx)
		{
			for (int32 previous = 0; previous < render::SpriteFont::s_CharCount - 1; previous++)
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
			LOG("FREETYPE: Failed to load glyph", core::LogLevel::Warning);
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
		metric->TexCoord = math::vecCast<float>(startPos[channel]);
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
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	render::TextureParameters params(false);
	params.minFilter = render::E_TextureFilterMode::Linear;
	params.magFilter = render::E_TextureFilterMode::Linear;
	params.wrapS = render::E_TextureWrapMode::ClampToEdge;
	params.wrapT = render::E_TextureWrapMode::ClampToEdge;

	render::TextureData* const texture = new render::TextureData(ivec2(texWidth, texHeight),
		render::E_ColorFormat::RGBA16f, 
		render::E_ColorFormat::RGBA, 
		render::E_DataType::Float);

	texture->Build();
	texture->SetParameters(params);
	pFont->m_pTexture = texture;

	render::T_FbLoc captureFBO;
	render::T_RbLoc captureRBO;

	api->GenFramebuffers(1, &captureFBO);
	api->GenRenderBuffers(1, &captureRBO);

	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);

	api->SetRenderbufferStorage(render::E_RenderBufferFormat::Depth24, ivec2(texWidth, texHeight));
	api->LinkRenderbufferToFbo(render::E_RenderBufferFormat::Depth24, captureRBO);
	api->LinkTextureToFbo2D(0, pFont->m_pTexture->GetLocation(), 0);

	ivec2 vpPos, vpSize;
	api->GetViewport(vpPos, vpSize);

	api->SetViewport(ivec2(0), ivec2(texWidth, texHeight));
	api->Clear(render::E_ClearFlag::Color | render::E_ClearFlag::Depth);

	AssetPtr<render::ShaderData> computeSdf = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("ComputeGlyphSDF.glsl"));
	api->SetShader(computeSdf.get());
	computeSdf->Upload("uSpread"_hash, static_cast<float>(asset->m_Spread));
	computeSdf->Upload("uHighRes"_hash, static_cast<float>(asset->m_HighRes));

	params.wrapS = render::E_TextureWrapMode::ClampToBorder;
	params.wrapT = render::E_TextureWrapMode::ClampToBorder;
	params.borderColor = vec4(0);

	api->SetBlendEnabled(true);
	api->SetBlendEquation(render::E_BlendEquation::Add);
	api->SetBlendFunction(render::E_BlendFactor::One, render::E_BlendFactor::One);

	//Render to Glyphs atlas
	FT_Set_Pixel_Sizes(face, 0, asset->m_FontSize * asset->m_HighRes);
	api->SetPixelUnpackAlignment(1);
	for (auto& character : characters)
	{
		auto metric = character.second;

		uint32 glyphIdx = FT_Get_Char_Index(face, metric->Character);
		if (FT_Load_Glyph(face, glyphIdx, FT_LOAD_DEFAULT))
		{
			LOG("FREETYPE: Failed to load glyph", core::LogLevel::Warning);
			continue;
		}
		if (!(face->glyph->format == FT_GLYPH_FORMAT_BITMAP))
		{
			if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
			{
				LOG("FREETYPE: Failed to render glyph", core::LogLevel::Warning);
				continue;
			}
		}

		uint32 width = face->glyph->bitmap.width;
		uint32 height = face->glyph->bitmap.rows;
		auto pTexture = new render::TextureData(ivec2(width, height), render::E_ColorFormat::R8, render::E_ColorFormat::Red, render::E_DataType::UByte);
		pTexture->Build(face->glyph->bitmap.buffer);
		pTexture->SetParameters(params);

		ivec2 res = ivec2(metric->Width - asset->m_Padding * 2, metric->Height - asset->m_Padding * 2);
		api->SetViewport(math::vecCast<int32>(metric->TexCoord) + ivec2(asset->m_Padding), res);
		computeSdf->Upload("uTex"_hash, static_cast<render::TextureData const*>(pTexture));
		computeSdf->Upload("uChannel"_hash, static_cast<int32>(metric->Channel));
		computeSdf->Upload("uResolution"_hash, math::vecCast<float>(res));
		render::RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<render::primitives::Quad>();

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
	api->SetViewport(vpPos, vpSize);

	api->DeleteRenderBuffers(1, &captureRBO);
	api->DeleteFramebuffers(1, &captureFBO);

	return pFont;
}


} // namespace pl
} // namespace et
