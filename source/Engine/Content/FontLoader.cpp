#include "stdafx.hpp"
#include "FontLoader.hpp"

#include "FileSystem/Entry.h"
#include "FileSystem/BinaryReader.hpp"
#include "TextureLoader.hpp"

#include <ft2build.h>
#include <freetype/freetype.h>
#include "TextureData.hpp"
#include "PrimitiveRenderer.hpp"
#include "ShaderData.hpp"

FontLoader::FontLoader()
{
}

FontLoader::~FontLoader()
{
}

SpriteFont* FontLoader::LoadContent(const std::string& assetFile)
{
	ivec2 logPos = Logger::GetCursorPosition();
	std::string loadingString = std::string("Loading Font: ") + assetFile + " . . .";

	LOG(loadingString + " . . . opening file          ", Info, false, logPos);

	File* input = new File( assetFile, nullptr );
	if(!input->Open( FILE_ACCESS_MODE::Read ))
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG("    Opening font file failed.", Warning);
		return nullptr;
	}
	std::vector<uint8> binaryContent = input->Read();
	std::string extension = input->GetExtension();
	delete input; 
	input = nullptr;
	if(binaryContent.size() == 0)
	{
		LOG(loadingString + " . . . FAILED!          ", Warning, false, logPos);
		LOG("    Font file is empty.", Warning);
		return nullptr;
	}

	SpriteFont* ret = nullptr;

	if (extension == "ttf")
	{
		LOG(loadingString + " . . . loading ttf          ", Info, false, logPos);
		ret = LoadTtf(binaryContent);
	}
	else if (extension == "fnt")
	{
		LOG(loadingString + " . . . loading fnt data          ", Info, false, logPos);
		ret = LoadFnt(binaryContent, assetFile);
	}
	else
	{
		LOG(loadingString + " . . . FAILED!         ", Warning, false, logPos);
		LOG("    Cannot load font with this extension. Supported exensions:", Warning);
		LOG("        ttf", Warning);
		LOG("        fnt", Warning);
		return nullptr;
	}

	if (!ret)
	{
		LOG(loadingString + " . . . FAILED!         ", Warning, false, logPos);
	}
	else
	{
		LOG(loadingString + " . . . SUCCESS!          ", Info, false, logPos);
	}
	return ret;
}

void FontLoader::Destroy(SpriteFont* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}

SpriteFont* FontLoader::LoadTtf(const std::vector<uint8>& binaryContent)
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
	for (int32 c = 0; c < SpriteFont::CHAR_COUNT-1; c++)
	{
		FontMetric* metric = &(pFont->GetMetric(static_cast<wchar_t>(c)));
		metric->Character = static_cast<wchar_t>(c);

		uint32 glyphIdx = FT_Get_Char_Index(face, c);

		if (pFont->m_UseKerning && glyphIdx)
		{
			for (int32 previous = 0; previous < SpriteFont::CHAR_COUNT - 1; previous++)
			{
				FT_Vector  delta;

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

	pFont->m_TextureWidth = std::max(std::max(maxPos[0].x, maxPos[1].x), std::max(maxPos[2].x, maxPos[3].x));
	pFont->m_TextureHeight = std::max(std::max(maxPos[0].y, maxPos[1].y), std::max(maxPos[2].y, maxPos[3].y));

	//Setup rendering
	TextureParameters params(false);
	params.wrapS = GL_CLAMP_TO_EDGE;
	params.wrapT = GL_CLAMP_TO_EDGE;
	
	pFont->m_pTexture = new TextureData(pFont->m_TextureWidth, pFont->m_TextureHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	pFont->m_pTexture->Build();
	pFont->m_pTexture->SetParameters(params);
	GLuint captureFBO, captureRBO;

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, pFont->m_TextureWidth, pFont->m_TextureHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pFont->m_pTexture->GetHandle(), 0);

	STATE->SetViewport(ivec2(0), ivec2(pFont->m_TextureWidth, pFont->m_TextureHeight));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ShaderData* pComputeSDF = ContentManager::Load<ShaderData>("Shaders/ComputeGlyphSDF.glsl");
	STATE->SetShader(pComputeSDF);
	glUniform1i(glGetUniformLocation(pComputeSDF->GetProgram(), "uTex"), 0);
	auto uChannel = glGetUniformLocation(pComputeSDF->GetProgram(), "uChannel");
	auto uResolution = glGetUniformLocation(pComputeSDF->GetProgram(), "uResolution");
	glUniform1f(glGetUniformLocation(pComputeSDF->GetProgram(), "uSpread"), (float)m_Spread);
	glUniform1f(glGetUniformLocation(pComputeSDF->GetProgram(), "uHighRes"), (float)m_HighRes);

	STATE->SetBlendEnabled(true);
	STATE->SetBlendEquation(GL_FUNC_ADD);
	STATE->SetBlendFunction(GL_ONE, GL_ONE);

	//Render to Glyphs atlas
	FT_Set_Pixel_Sizes(face, 0, m_FontSize * m_HighRes);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
		auto pTexture = new TextureData(width, height, GL_RED, GL_RED, GL_UNSIGNED_BYTE);
		pTexture->Build(face->glyph->bitmap.buffer);
		pTexture->SetParameters(params);

		ivec2 res = ivec2(metric->Width - totPadding * 2, metric->Height - totPadding * 2);
		STATE->SetViewport(etm::vecCast<int32>(metric->TexCoord)+ivec2(totPadding), res);
		STATE->LazyBindTexture(0, GL_TEXTURE_2D, pTexture->GetHandle());
		glUniform1i(uChannel, metric->Channel);
		glUniform2f(uResolution, (float)res.x, (float)res.y);
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

		delete pTexture;

		//modify texture coordinates after rendering sprites
		metric->TexCoord = metric->TexCoord / vec2((float)pFont->m_TextureWidth, (float)pFont->m_TextureHeight);
	}

	//Cleanup
	STATE->SetBlendEnabled(false);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	STATE->BindFramebuffer(0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	STATE->SetViewport(ivec2(0), WINDOW.Dimensions);

	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);

	return pFont;
}

SpriteFont* FontLoader::LoadFnt(const std::vector<uint8>& binaryContent, const std::string& assetFile)
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
	pFont->m_TextureWidth = pBinReader->Read<uint16>();
	pFont->m_TextureHeight = pBinReader->Read<uint16>();
	auto pagecount = pBinReader->Read<uint16>();
	if (pagecount > 1) LOG("SpriteFont::Load > SpriteFont(.fnt): Only one texture per font allowed", Warning);
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
	if (pn.size() == 0) LOG("SpriteFont::Load > SpriteFont(.fnt): Invalid Font Sprite [Empty]", Warning);
	auto filepath = assetFile.substr(0, assetFile.rfind('/') + 1);

	TextureLoader* pTL = ContentManager::GetLoader<TextureLoader, TextureData>();
	pTL->ForceResolution(true);
	pFont->m_pTexture = ContentManager::Load<TextureData>(filepath + pn);
	pTL->ForceResolution(false);
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
		auto posChar = pBinReader->GetBufferPosition();
		auto charId = (wchar_t)(pBinReader->Read<uint32>());
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
			metric->TexCoord = vec2((float)xPos / (float)pFont->m_TextureWidth
				, (float)yPos / (float)pFont->m_TextureHeight);
			pBinReader->SetBufferPosition(posChar + 20);
		}
	}
	delete pBinReader;

	pFont->m_IsCachedFont = true;

	return pFont;
}
