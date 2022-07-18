#include "stdafx.h"
#include "EditableSdfFont.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <stb/stb_image_write.h>

#include <cctype>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>
#include <EtCore/IO/BinaryWriter.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtPipeline/PipelineCtx.h>
#include <EtPipeline/Import/TextureCompression.h>


namespace et {
namespace pl {


//=========================
// Editable Sdf Font Asset
//=========================


// reflection
RTTR_REGISTRATION
{

	BEGIN_REGISTER_CLASS(EditableSdfFontAsset::Charset, "font charset")
		.property("start", &EditableSdfFontAsset::Charset::m_Start)
		.property("end", &EditableSdfFontAsset::Charset::m_End)
	END_REGISTER_CLASS(EditableSdfFontAsset::Charset);

	BEGIN_REGISTER_CLASS(EditableSdfFontAsset, "editable sdf font asset")
		.property("size", &EditableSdfFontAsset::m_FontSize)
		.property("padding", &EditableSdfFontAsset::m_Padding)
		.property("spread", &EditableSdfFontAsset::m_Spread)
		.property("highres", &EditableSdfFontAsset::m_HighRes)
		.property("EM per 100 weight", &EditableSdfFontAsset::m_EmPer100Weight)
		.property("char sets", &EditableSdfFontAsset::m_CharSets)
	END_REGISTER_CLASS_POLYMORPHIC(EditableSdfFontAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableSdfFontAsset) // force the asset class to be linked as it is only used in reflection


// static
std::string const EditableSdfFontAsset::s_FontFileExt("ttf");


//--------------------------------------
// EditableSdfFontAsset::LoadFromMemory
//
bool EditableSdfFontAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	ET_ASSERT(core::FileUtil::ExtractExtension(m_Asset->GetName()) == "ttf");

	// Create shader data
	SetData(LoadTtf(data));
	gui::SdfFont* const fontData = GetData();

	if (fontData == nullptr)
	{
		ET_LOG_E(ET_CTX_PIPELINE, "EditableSdfFontAsset::LoadFromMemory > Loading font failed!");
		return false;
	}

	return true;
}

//--------------------------------------------------
// EditableSdfFontAsset::SetupRuntimeAssetsInternal
//
void EditableSdfFontAsset::SetupRuntimeAssetsInternal()
{
	// #todo: replace file extension with bmf once assets aren't referenced with load data name
	gui::SdfFontAsset* const mainAsset = new gui::SdfFontAsset(*static_cast<gui::SdfFontAsset*>(m_Asset));
	ET_ASSERT(core::FileUtil::ExtractExtension(mainAsset->GetName()) == s_FontFileExt);

	m_RuntimeAssets.emplace_back(mainAsset, true);

	render::TextureAsset* const textureAsset = new render::TextureAsset();
	textureAsset->SetName(core::FileUtil::RemoveExtension(mainAsset->GetName()) + "." + render::TextureFormat::s_TextureFileExt);
	textureAsset->SetPath(mainAsset->GetPath());
	textureAsset->SetPackageId(mainAsset->GetPackageId());

	textureAsset->m_ForceResolution = true;
	PopulateTextureParams(textureAsset->m_Parameters);

	m_RuntimeAssets.emplace_back(textureAsset, true);

	mainAsset->SetReferenceIds(std::vector<core::HashString>({ textureAsset->GetId() }));
}

//----------------------------------------
// EditableSdfFontAsset::GenerateInternal
//
bool EditableSdfFontAsset::GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath)
{
	ET_UNUSED(buildConfig);
	ET_UNUSED(dbPath);

	// Load the font from TTF
	//------------------------
	gui::SdfFont* const font = LoadTtf(m_Asset->GetLoadData());

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
		else if (ext == render::TextureFormat::s_TextureFileExt)
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

	if (!GenerateBinFontData(fontData->m_GeneratedData, font, textureData->m_Asset->GetPath() + textureData->m_Asset->GetName()))
	{
		ET_ASSERT(false, "failed to generate font data");
		return false;
	}

	return true;
}

//---------------------------------
// EditableSdfFontAsset::LoadTtf
//
// Rasterizes a ttf font into a SDF texture and generates SdfFont data from it
//
gui::SdfFont* EditableSdfFontAsset::LoadTtf(const std::vector<uint8>& binaryContent)
{
	// init
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		ET_LOG_E(ET_CTX_PIPELINE, "FREETYPE: Could not init FreeType Library");
	}

	FT_Face face;
	if (FT_New_Memory_Face(ft, binaryContent.data(), (FT_Long)binaryContent.size(), 0, &face))
	{
		ET_LOG_E(ET_CTX_PIPELINE, "FREETYPE: Failed to load font");
	}

	// parse basic font parameters
	FT_Set_Pixel_Sizes(face, 0, m_FontSize);

	gui::SdfFont* const font = new gui::SdfFont();
	font->m_FontSize = static_cast<int16>(m_FontSize);
	font->m_FontFamily = std::string(face->family_name);
	std::transform(font->m_FontFamily.begin(), font->m_FontFamily.end(), font->m_FontFamily.begin(),
		[](unsigned char c)
		{
			return std::tolower(c);
		});

	font->m_IsItalic = (face->style_flags & FT_STYLE_FLAG_ITALIC);

	{
		TT_OS2* const fontTable = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face, FT_SFNT_OS2));
		if ((fontTable != nullptr) && (fontTable->usWeightClass != 0))
		{
			font->m_Weight = static_cast<gui::SdfFont::E_Weight>(fontTable->usWeightClass);
		}
		else
		{
			font->m_Weight = (face->style_flags & FT_STYLE_FLAG_BOLD) ? gui::SdfFont::E_Weight::Bold : gui::SdfFont::E_Weight::Normal;
		}
	}

	font->m_LineHeight = static_cast<uint16>(face->size->metrics.height >> 6);
	font->m_Baseline = font->m_LineHeight - static_cast<uint16>(face->size->metrics.ascender >> 6);

	static float const s_ScaleFactor = 1.f;
	font->m_Underline = static_cast<int32>(FT_MulFix(face->underline_position, face->size->metrics.y_scale) * s_ScaleFactor / float(1 << 6));
	font->m_UnderlineThickness = FT_MulFix(face->underline_thickness, face->size->metrics.y_scale) * s_ScaleFactor / float(1 << 6);
	font->m_UnderlineThickness = std::max(font->m_UnderlineThickness, 1.0f);

	font->m_UseKerning = FT_HAS_KERNING(face) != 0;

	// Atlassing helper variables
	ivec2 startPos[4] = { ivec2(0), ivec2(0), ivec2(0), ivec2(0) };
	ivec2 maxPos[4] = { ivec2(0), ivec2(0), ivec2(0), ivec2(0) };
	bool horizontal = false; // Direction this pass expands the map in (internal moves are !horizontal)
	uint32 posCount = 1; // internal move count in this pass
	uint32 curPos = 0; // internal move count
	uint32 channel = 0; // channel to add to

	uint32 const totPadding = m_Padding + m_Spread;

	// Load individual character metrics
	std::unordered_map<char32, gui::SdfFont::Metric*> characters;
	for (Charset const& charset : m_CharSets)
	{
		bool hasCharset = false; // for trimming the charset to the smallest possible range
		char32 lastValidChar = static_cast<char32>(-1);

		for (char32 character = charset.m_Start; character <= charset.m_End; ++character)
		{
			uint32 const glyphIdx = FT_Get_Char_Index(face, static_cast<FT_ULong>(character));
			if ((glyphIdx == 0) 
				&& (character != 0) // we want to make sure we render a glyph for the null character
				&& (character != static_cast<char32>('x'))) // fonts should have a metric for lower case x
			{
				if (hasCharset)
				{
					font->GetMetric(character)->m_Character = character;
				}

				continue;
			}

			lastValidChar = character;
			if (!hasCharset)
			{
				font->m_CharSets.push_back(gui::SdfFont::Charset(character, charset.m_End));
				font->m_CharSets.back().m_Characters.resize(charset.m_End - character + 1);
				hasCharset = true;
			}

			gui::SdfFont::Metric* const metric = font->GetMetric(character);
			metric->m_Character = character;

			if (font->m_UseKerning && glyphIdx)
			{
				for (Charset const& innerSet : m_CharSets)
				{
					for (char32 innerChar = innerSet.m_Start; innerChar <= innerSet.m_End; ++innerChar)
					{
						FT_Vector delta;

						uint32 prevIdx = FT_Get_Char_Index(face, static_cast<FT_ULong>(innerChar));
						FT_Get_Kerning(face, prevIdx, glyphIdx, FT_KERNING_DEFAULT, &delta);

						if (delta.x) // we ignore vertical kerning because the BMF format doesn't support it
						{
							metric->m_Kerning[innerChar] = vec2(static_cast<float>(delta.x) / gui::SdfFontAsset::s_KerningAdjustment, 0.f);
						}
					}
				}
			}

			if (FT_Load_Glyph(face, glyphIdx, FT_LOAD_DEFAULT))
			{
				ET_ASSERT(false, "FREETYPE: Failed to load glyph");
				continue;
			}

			// for the SDF we pad out the glyph bitmaps, this needs to be accounted for in the positioning for the characters
			uint32 const width = face->glyph->bitmap.width + totPadding * 2;
			uint32 const height = face->glyph->bitmap.rows + totPadding * 2;

			metric->m_Width = static_cast<uint16>(width);
			metric->m_Height = static_cast<uint16>(height);
			metric->m_OffsetX = static_cast<int16>(face->glyph->bitmap_left) - static_cast<int16>(totPadding);
			metric->m_OffsetY = -(static_cast<int16>(face->glyph->bitmap_top) + static_cast<int16>(totPadding));
			metric->m_AdvanceX = static_cast<float>(face->glyph->advance.x) / 64.f;

			// Generate atlas coordinates
			metric->m_Page = 0;
			metric->m_Channel = (uint8)channel;
			metric->m_TexCoord = math::vecCast<float>(startPos[channel]);
			if (horizontal)
			{
				maxPos[channel].y = std::max(maxPos[channel].y, startPos[channel].y + static_cast<int32>(height));
				startPos[channel].y += height;
				maxPos[channel].x = std::max(maxPos[channel].x, startPos[channel].x + static_cast<int32>(width));
			}
			else
			{
				maxPos[channel].x = std::max(maxPos[channel].x, startPos[channel].x + static_cast<int32>(width));
				startPos[channel].x += width;
				maxPos[channel].y = std::max(maxPos[channel].y, startPos[channel].y + static_cast<int32>(height));
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

			metric->m_IsValid = true;

			characters[character] = metric;
		}

		if (hasCharset)
		{
			gui::SdfFont::Charset& genSet = font->m_CharSets.back();
			genSet.m_End = lastValidChar;
			genSet.m_Characters.resize(genSet.m_End - genSet.m_Start + 1);
		}
	}

	font->m_SdfSize = m_Spread;
	float const thresholdEm = (static_cast<float>(m_Spread) * 2.f) / static_cast<float>(m_FontSize);
	font->m_ThresholdPerWeight = (m_EmPer100Weight / thresholdEm) * 0.01f;


	// render the atlas
	//------------------

	int32 const texWidth = std::max(std::max(maxPos[0].x, maxPos[1].x), std::max(maxPos[2].x, maxPos[3].x));
	int32 const texHeight = std::max(std::max(maxPos[0].y, maxPos[1].y), std::max(maxPos[2].y, maxPos[3].y));

	//Setup rendering
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	render::TextureParameters params;
	PopulateTextureParams(params);

	{
		UniquePtr<render::TextureData> texture = Create<render::TextureData>(render::E_ColorFormat::RGBA16f, ivec2(texWidth, texHeight));
		texture->AllocateStorage();
		texture->SetParameters(params);
		font->m_Texture = std::move(texture);
	}

	render::T_FbLoc captureFBO;
	render::T_RbLoc captureRBO;

	api->GenFramebuffers(1, &captureFBO);
	api->GenRenderBuffers(1, &captureRBO);

	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);

	api->SetRenderbufferStorage(render::E_RenderBufferFormat::Depth24, ivec2(texWidth, texHeight));
	api->LinkRenderbufferToFbo(render::E_RenderBufferFormat::Depth24, captureRBO);
	api->LinkTextureToFbo2D(0, font->m_Texture->GetLocation(), 0);

	ivec2 vpPos, vpSize;
	api->GetViewport(vpPos, vpSize);

	api->SetViewport(ivec2(0), ivec2(texWidth, texHeight));
	api->Clear(render::E_ClearFlag::CF_Color | render::E_ClearFlag::CF_Depth);

	AssetPtr<render::ShaderData> const computeSdf = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(
		core::HashString("Shaders/ComputeGlyphSDF.glsl"));
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
		gui::SdfFont::Metric& metric = *character.second;

		uint32 const glyphIdx = FT_Get_Char_Index(face, metric.m_Character);
		if (FT_Load_Glyph(face, glyphIdx, FT_LOAD_DEFAULT))
		{
			ET_LOG_W(ET_CTX_PIPELINE, "FREETYPE: Failed to load glyph");
			continue;
		}

		if (!(face->glyph->format == FT_GLYPH_FORMAT_BITMAP))
		{
			if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
			{
				ET_LOG_W(ET_CTX_PIPELINE, "FREETYPE: Failed to render glyph");
				continue;
			}
		}

		uint32 const width = face->glyph->bitmap.width;
		uint32 const height = face->glyph->bitmap.rows;
		UniquePtr<render::TextureData> texture = Create<render::TextureData>(render::E_ColorFormat::R8, ivec2(width, height));
		texture->UploadData(face->glyph->bitmap.buffer, render::E_ColorFormat::Red, render::E_DataType::UByte, 0u);
		texture->SetParameters(params);

		ivec2 const res = ivec2(metric.m_Width, metric.m_Height) - ivec2(m_Padding * 2);
		api->SetViewport(math::vecCast<int32>(metric.m_TexCoord) + ivec2(m_Padding), res);
		computeSdf->Upload("uTex"_hash, static_cast<render::TextureData const*>(texture.Get()));
		computeSdf->Upload("uChannel"_hash, static_cast<int32>(metric.m_Channel));
		computeSdf->Upload("uResolution"_hash, math::vecCast<float>(res));
		render::RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<render::primitives::Quad>();

		//modify texture coordinates after rendering sprites
		metric.m_TexCoord = metric.m_TexCoord / math::vecCast<float>(ivec2(texWidth, texHeight));
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

	return font;
}

//---------------------------------------------
// EditableSdfFontAsset::PopulateTextureParams
//
// Texture parameters to be used by the glyph atlas
//
void EditableSdfFontAsset::PopulateTextureParams(render::TextureParameters& params) const
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

//-------------------------------------------
// EditableSdfFontAsset::GenerateBinFontData
//
// Font asset descriptor written in this file format: http://www.angelcode.com/products/bmfont/doc/file_format.html
//  - format modifications: 
//    * each metric contains a byte indicating if the metric is valid or not after the character ID
//	  * added uint16 to block 1 for precise font weight
//	  * added int16 to block 2 for Underline position
//	  * added float to block 2 for Underline thickness
//	  * added float to block 2 for Sdf Size
//
bool EditableSdfFontAsset::GenerateBinFontData(std::vector<uint8>& data, gui::SdfFont const* const font, std::string const& atlasName)
{
	// struct definitions
	//--------------------

	static size_t const s_BlockHeaderSize = 5u;
	auto const writeBlockHeader = [](core::BinaryWriter& binWriter, uint8 const id, uint32 const size)
		{
			binWriter.Write(id);
			binWriter.Write(size);
		};

	static size_t const s_CharacterSize = 21u;

	static size_t const s_KerningPairSize = 10u;
	struct KerningPair
	{
		KerningPair(char32 const first, char32 const second, int16 const amount)
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
	uint32 const block1Size = static_cast<uint32>(16u + font->m_FontFamily.size() + 1u); // +1 because string is null terminated

	// 2
	uint32 const block2Size = 29u;

	// 3
	uint32 const block3Size = static_cast<uint32>(atlasName.size() + 1u); // +1 because string is null terminated

	// 4, 5

	std::vector<gui::SdfFont::Metric> metrics;
	std::vector<KerningPair> kerningPairs;
	for (gui::SdfFont::Charset const& charset : font->m_CharSets)
	{
		for (gui::SdfFont::Metric const& metric : charset.m_Characters)
		{
			metrics.push_back(metric);
			for (auto const& el : metric.m_Kerning)
			{
				kerningPairs.emplace_back(metric.m_Character, el.first, static_cast<int16>(el.second.x * gui::SdfFontAsset::s_KerningAdjustment));
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
	binWriter.Write<uint8>(font->m_IsItalic ? 1u << 2 : 0u);  // italic flag at bit 2
	binWriter.Write(static_cast<uint16>(font->m_Weight)); // non standard
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
	binWriter.WriteNullString(font->m_FontFamily);

	// Block 2
	//---------
	writeBlockHeader(binWriter, 2u, block2Size);

	binWriter.Write<uint16>(font->m_LineHeight); 
	binWriter.Write<uint16>(font->m_Baseline); 
	binWriter.Write<int16>(font->m_Underline);  // non-standard
	binWriter.Write<float>(font->m_UnderlineThickness); // non-standard

	ivec2 const res = font->GetAtlas()->GetResolution();
	binWriter.Write(static_cast<uint16>(res.x)); // scaleW
	binWriter.Write(static_cast<uint16>(res.y)); // scaleH
	binWriter.Write<uint16>(1u); // pages
	binWriter.Write<uint8>(0u); // bitField
	binWriter.Write<uint8>(0u); // alphaChnl
	binWriter.Write<uint8>(0u); // redChnl
	binWriter.Write<uint8>(0u); // greenChnl
	binWriter.Write<uint8>(0u); // blueChnl
	binWriter.Write(font->m_SdfSize); // non-standard
	binWriter.Write(font->m_ThresholdPerWeight); // non-standard

	// Block 3
	//---------
	writeBlockHeader(binWriter, 3u, block3Size);

	binWriter.WriteNullString(atlasName);

	// Block 4
	//---------
	writeBlockHeader(binWriter, 4u, block4Size);

	for (gui::SdfFont::Metric const& metric : metrics)
	{
		binWriter.Write(static_cast<uint32>(metric.m_Character)); // id
		binWriter.Write(static_cast<uint8>(metric.m_IsValid)); // non-standard

		ivec2 pos = math::vecCast<int32>(metric.m_TexCoord * math::vecCast<float>(res));
		binWriter.Write(static_cast<uint16>(pos.x)); // x
		binWriter.Write(static_cast<uint16>(pos.y)); // y

		binWriter.Write<uint16>(metric.m_Width); // width
		binWriter.Write<uint16>(metric.m_Height); // height
		binWriter.Write<int16>(metric.m_OffsetX); // xoffset
		binWriter.Write<int16>(metric.m_OffsetY); // yoffset
		binWriter.Write(static_cast<int16>(metric.m_AdvanceX)); // advance

		binWriter.Write<uint8>(metric.m_Page); // page

		switch (metric.m_Channel) // chnl
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
			ET_ASSERT(false, "undefined channel (%u) for character %c", metric.m_Channel, metric.m_Character);
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

//-------------------------------------------
// EditableSdfFontAsset::GenerateTextureData
//
bool EditableSdfFontAsset::GenerateTextureData(std::vector<uint8>& data, render::TextureData const* const texture)
{
	// Prepare raster image with correct size
	ivec2 const dim = texture->GetResolution();
	RasterImage image(dim.x, dim.y);
	image.AllocatePixels();

	// read pixels from GPU
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();
	api->GetTextureData(*texture, 0u, render::E_ColorFormat::RGBA, render::E_DataType::UByte, reinterpret_cast<void*>(image.GetPixels()));

	// convert to output format
	return TextureCompression::WriteTextureFile(data,
		image, 
		TextureCompression::E_Setting::SdfFont,
		TextureCompression::E_Quality::Ultra,
		true,
		render::TextureFormat::E_Srgb::None,
		0u,
		true,
		false);
}


} // namespace pl
} // namespace et
