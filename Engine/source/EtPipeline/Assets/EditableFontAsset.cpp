#include "stdafx.h"
#include "EditableFontAsset.h"

#include <ft2build.h>
#include <freetype/freetype.h>

#include <stb/stb_image_write.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>
#include <EtCore/IO/BinaryWriter.h>

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
		.property("size", &EditableFontAsset::m_FontSize)
		.property("padding", &EditableFontAsset::m_Padding)
		.property("spread", &EditableFontAsset::m_Spread)
		.property("highres", &EditableFontAsset::m_HighRes)
	END_REGISTER_CLASS_POLYMORPHIC(EditableFontAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableFontAsset) // force the asset class to be linked as it is only used in reflection


// static
std::string const EditableFontAsset::s_FontFileExt("ttf");
std::string const EditableFontAsset::s_TextureFileExt("tga");


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

//-----------------------------------------------
// EditableFontAsset::SetupRuntimeAssetsInternal
//
void EditableFontAsset::SetupRuntimeAssetsInternal()
{
	// #todo: replace file extension with bmf once assets aren't referenced with load data name
	render::FontAsset* const mainAsset = new render::FontAsset(*static_cast<render::FontAsset*>(m_Asset));
	ET_ASSERT(core::FileUtil::ExtractExtension(mainAsset->GetName()) == s_FontFileExt);

	m_RuntimeAssets.emplace_back(mainAsset, true);

	render::TextureAsset* const textureAsset = new render::TextureAsset();
	textureAsset->SetName(core::FileUtil::RemoveExtension(mainAsset->GetName()) + "." + s_TextureFileExt);
	textureAsset->SetPath(mainAsset->GetPath());
	textureAsset->SetPackageId(mainAsset->GetPackageId());

	textureAsset->m_UseSrgb = false;
	textureAsset->m_ForceResolution = true;
	PopulateTextureParams(textureAsset->m_Parameters);

	m_RuntimeAssets.emplace_back(textureAsset, true);

	mainAsset->SetReferenceIds(std::vector<core::HashString>({ textureAsset->GetId() }));
}

//-------------------------------------
// EditableFontAsset::GenerateInternal
//
bool EditableFontAsset::GenerateInternal(BuildConfiguration const& buildConfig)
{
	UNUSED(buildConfig);

	// Load the font from TTF
	//------------------------
	render::SpriteFont* const font = LoadTtf(m_Asset->GetLoadData());

	// Write Data
	//--------------------
	RuntimeAssetData* fontData = nullptr;
	RuntimeAssetData* textureData = nullptr;
	for (RuntimeAssetData& data : m_RuntimeAssets)
	{
		data.m_HasGeneratedData = true;

		std::string const ext = core::FileUtil::ExtractExtension(data.m_Asset->GetName());
		if (ext == s_FontFileExt)
		{
			fontData = &data;
		}
		else if (ext == s_TextureFileExt)
		{
			textureData = &data;
		}
		else
		{
			ET_ASSERT(false, "unexpected file extension");
			data.m_HasGeneratedData = false;
		}
	}

	if (fontData == nullptr)
	{
		ET_ASSERT(false, "couldn't find font runtime asset data");
		return false;
	}

	if (textureData == nullptr)
	{
		ET_ASSERT(false, "couldn't find texture runtime asset data");
		return false;
	}

	if (!GenerateTextureData(textureData->m_GeneratedData, font->GetAtlas()))
	{
		ET_ASSERT(false, "failed to generate texture data");
		return false;
	}

	if (!GenerateBinFontData(fontData->m_GeneratedData, font, textureData->m_Asset->GetName()))
	{
		ET_ASSERT(false, "failed to generate font data");
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

	FT_Set_Pixel_Sizes(face, 0, m_FontSize);

	render::SpriteFont* pFont = new render::SpriteFont();
	pFont->m_FontSize = static_cast<int16>(m_FontSize);
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

				if (delta.x) // we ignore vertical kerning because the BMF format doesn't support it
				{
					metric->Kerning[static_cast<wchar_t>(previous)] = vec2((float)delta.x / render::FontAsset::s_KerningAdjustment, 0.f); 
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

	render::TextureParameters params;
	PopulateTextureParams(params);

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
	computeSdf->Upload("uSpread"_hash, static_cast<float>(m_Spread));
	computeSdf->Upload("uHighRes"_hash, static_cast<float>(m_HighRes));

	params.wrapS = render::E_TextureWrapMode::ClampToBorder;
	params.wrapT = render::E_TextureWrapMode::ClampToBorder;

	api->SetBlendEnabled(true);
	api->SetBlendEquation(render::E_BlendEquation::Add);
	api->SetBlendFunction(render::E_BlendFactor::One, render::E_BlendFactor::One);

	//Render to Glyphs atlas
	FT_Set_Pixel_Sizes(face, 0, m_FontSize * m_HighRes);
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

		ivec2 res = ivec2(metric->Width - m_Padding * 2, metric->Height - m_Padding * 2);
		api->SetViewport(math::vecCast<int32>(metric->TexCoord) + ivec2(m_Padding), res);
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

//------------------------------------------
// EditableFontAsset::PopulateTextureParams
//
// Texture parameters to be used by the glyph atlas
//
void EditableFontAsset::PopulateTextureParams(render::TextureParameters& params) const
{
	params.minFilter = render::E_TextureFilterMode::Linear;
	params.magFilter = render::E_TextureFilterMode::Linear;
	params.mipFilter = render::E_TextureFilterMode::Nearest;

	params.wrapS = render::E_TextureWrapMode::ClampToEdge;
	params.wrapT = render::E_TextureWrapMode::ClampToEdge;

	params.borderColor = vec4(0.f);

	params.genMipMaps = false;

	params.isDepthTex = false;
	params.compareMode = render::E_TextureCompareMode::None;
}

//----------------------------------------
// EditableFontAsset::GenerateBinFontData
//
// Font asset descriptor written in this file format: http://www.angelcode.com/products/bmfont/doc/file_format.html
//
bool EditableFontAsset::GenerateBinFontData(std::vector<uint8>& data, render::SpriteFont const* const font, std::string const& atlasName)
{
	// struct definitions
	//--------------------

	static size_t const s_BlockHeaderSize = 5u;
	auto const writeBlockHeader = [](core::BinaryWriter& binWriter, uint8 const id, uint32 const size)
		{
			binWriter.Write(id);
			binWriter.Write(size);
		};

	static size_t const s_CharacterSize = 20u;

	static size_t const s_KerningPairSize = 10u;
	struct KerningPair
	{
		KerningPair(wchar_t const first, wchar_t const second, int16 const amount)
			: m_First(static_cast<uint32>(first)), m_Second(static_cast<uint32>(second)), m_Amount(amount)
		{ }

		uint32 m_First;
		uint32 m_Second;
		int16 m_Amount;
	};

	// can't write directly because compiler might change sizes of structs
	auto const writeKerningPair = [](core::BinaryWriter& binWriter, KerningPair const& pair)
		{
			binWriter.Write(pair.m_First);
			binWriter.Write(pair.m_Second);
			binWriter.Write(pair.m_Amount);
		};

	// Determine block sizes
	//-----------------------
	// 1
	uint32 const block1Size = static_cast<uint32>(14u + font->m_FontName.size() + 1u); // +1 because string is null terminated

	// 2
	uint32 const block2Size = 15u;

	// 3
	uint32 const block3Size = static_cast<uint32>(atlasName.size() + 1u); // +1 because string is null terminated

	// 4, 5

	std::vector<render::FontMetric> metrics;
	std::vector<KerningPair> kerningPairs;
	for (size_t charIdx = 0u; charIdx < render::SpriteFont::s_CharCount; ++charIdx)
	{
		render::FontMetric const& metric = font->m_CharTable[charIdx];
		if (metric.IsValid)
		{
			metrics.push_back(metric);
			for (auto const& el : metric.Kerning)
			{
				kerningPairs.emplace_back(metric.Character, el.first, static_cast<int16>(el.second.x * render::FontAsset::s_KerningAdjustment));
			}
		}
	}

	uint32 const block4Size = static_cast<uint32>(metrics.size() * s_CharacterSize); // should work differently in the future
	uint32 const block5Size = static_cast<uint32>(kerningPairs.size() * s_KerningPairSize);

	// if there are no kerning pairs we don't write block 5
	size_t const block5TotalSize = (kerningPairs.empty()) ? 0u : s_BlockHeaderSize + static_cast<size_t>(block5Size);

	// Format
	//--------

	core::BinaryWriter binWriter(data);
	binWriter.FormatBuffer(4u + // file header
		s_BlockHeaderSize + static_cast<size_t>(block1Size) +
		s_BlockHeaderSize + static_cast<size_t>(block2Size) +
		s_BlockHeaderSize + static_cast<size_t>(block3Size) +
		s_BlockHeaderSize + static_cast<size_t>(block4Size) +
		block5TotalSize);

	// File Header
	//-------------
	binWriter.Write('B');
	binWriter.Write('M');
	binWriter.Write('F');
	binWriter.Write<int8>(3); // file format version

	// Block 1
	//---------
	writeBlockHeader(binWriter, 1u, block1Size);

	binWriter.Write<int16>(font->GetFontSize());
	binWriter.Write<uint8>(0u); // bitField
	binWriter.Write<uint8>(0u); // charSet
	binWriter.Write<uint16>(0u); // stretchH
	binWriter.Write<uint8>(0u); // aa

	uint8 const padding = static_cast<uint8>(m_Padding);
	binWriter.Write(padding); // paddingUp
	binWriter.Write(padding); // paddingRight
	binWriter.Write(padding); // paddingDown
	binWriter.Write(padding); // paddingLeft

	binWriter.Write<uint8>(0u); // spacingHoriz
	binWriter.Write<uint8>(0u); // spacingVert
	binWriter.Write<uint8>(0u); // outline
	binWriter.WriteNullString(font->m_FontName);

	// Block 2
	//---------
	writeBlockHeader(binWriter, 2u, block2Size);

	binWriter.Write<uint16>(0u); // lineHeight
	binWriter.Write<uint16>(0u); // base

	ivec2 const res = font->GetAtlas()->GetResolution();
	binWriter.Write(static_cast<uint16>(res.x)); // scaleW
	binWriter.Write(static_cast<uint16>(res.y)); // scaleH
	binWriter.Write<uint16>(1u); // pages
	binWriter.Write<uint8>(0u); // bitField
	binWriter.Write<uint8>(0u); // alphaChnl
	binWriter.Write<uint8>(0u); // redChnl
	binWriter.Write<uint8>(0u); // greenChnl
	binWriter.Write<uint8>(0u); // blueChnl

	// Block 3
	//---------
	writeBlockHeader(binWriter, 3u, block3Size);

	binWriter.WriteNullString(atlasName);

	// Block 4
	//---------
	writeBlockHeader(binWriter, 4u, block4Size);

	for (render::FontMetric const& metric : metrics)
	{
		binWriter.Write(static_cast<uint32>(metric.Character)); // id

		ivec2 pos = math::vecCast<int32>(metric.TexCoord * math::vecCast<float>(res));
		binWriter.Write(static_cast<uint16>(pos.x)); // x
		binWriter.Write(static_cast<uint16>(pos.y)); // y

		binWriter.Write<uint16>(metric.Width); // width
		binWriter.Write<uint16>(metric.Height); // height
		binWriter.Write<int16>(metric.OffsetX); // xoffset
		binWriter.Write<int16>(metric.OffsetY); // yoffset
		binWriter.Write(static_cast<int16>(metric.AdvanceX)); // advance

		binWriter.Write<uint8>(metric.Page); // page

		switch (metric.Channel) // chnl
		{
		case 0u:
			binWriter.Write<uint8>(4u);
			break;

		case 1u:
			binWriter.Write<uint8>(2u);
			break;

		case 2u:
			binWriter.Write<uint8>(1u);
			break;

		case 3u:
			binWriter.Write<uint8>(8u);
			break;

		default:
			ET_ASSERT(false, "undefined channel (%u) for character %c", metric.Channel, metric.Character);
			return false;
		}
	}

	// Block 5
	//---------
	if (!kerningPairs.empty())
	{
		writeBlockHeader(binWriter, 5u, block5Size);

		for (KerningPair const& pair : kerningPairs)
		{
			writeKerningPair(binWriter, pair);
		}
	}

	return true;
}

//----------------------------------------
// EditableFontAsset::GenerateTextureData
//
bool EditableFontAsset::GenerateTextureData(std::vector<uint8>& data, render::TextureData const* const texture)
{
	// Make the BYTE array, factor of 4 because it's RBGA
	ivec2 const dim = texture->GetResolution();
	uint8* pixels = new uint8[4u * dim.x * dim.y];

	// read pixels from GPU
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();
	api->GetTextureData(*texture, render::E_ColorFormat::RGBA, render::E_DataType::UByte, pixels);

	// convert to output format
	stbi_flip_vertically_on_write(false);
	bool success = stbi_write_tga_to_func([](void* context, void* data, int size)
		{
			std::vector<uint8>& outData = *reinterpret_cast<std::vector<uint8>*>(context);
			uint8* const fmtData = reinterpret_cast<uint8*>(data);
			outData.insert(outData.end(), &fmtData[0], &fmtData[size]);
		},
		&data,
		dim.x,
		dim.y,
		4,
		pixels);

	// free resources
	delete[] pixels;
	return success;
}


} // namespace pl
} // namespace et
