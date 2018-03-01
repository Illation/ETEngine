#include "stdafx.hpp"
#include "FontLoader.hpp"

#include "FileSystem/Entry.h"
#include "FileSystem/BinaryReader.hpp"
#include "TextureLoader.hpp"

#include <ft2build.h>
#include <freetype/freetype.h>
//#include FT_FREETYPE_H  

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

	std::map<char, FontMetric> characters;
	for (char c = 0; c < SpriteFont::CHAR_COUNT; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			LOG("FREETYPE: Failed to load glyph", Warning);
			continue;
		}
	}

	//#todo to be filled out
	//pFont->m_TextureWidth;
	//pFont->m_TextureHeight;
	//pFont->m_pTexture;

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	LOG("Loading ttf files is not implemented yet", Warning);
	return nullptr;
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

	return pFont;
}
