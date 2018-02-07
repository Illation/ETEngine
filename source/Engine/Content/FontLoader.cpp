#include "stdafx.hpp"
#include "FontLoader.hpp"

#include "FileSystem/Entry.h"
#include "FileSystem/BinaryReader.hpp"
#include "TextureLoader.hpp"

FontLoader::FontLoader()
{
}

FontLoader::~FontLoader()
{
}

SpriteFont* FontLoader::LoadContent(const std::string& assetFile)
{
	using namespace std;
	cout << "Loading Font: " << assetFile << " . . . ";

	File* input = new File( assetFile, nullptr );
	if(!input->Open( FILE_ACCESS_MODE::Read ))
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Opening font descriptor file failed." << endl;
		return nullptr;
	}
	std::vector<uint8> binaryContent = input->Read();
	delete input; 
	input = nullptr;
	if(binaryContent.size() == 0)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Font descriptor is empty." << endl;
		return nullptr;
	}

	auto pBinReader = new BinaryReader(); //Prevent memory leaks
	pBinReader->Open(binaryContent);

	if (!pBinReader->Exists())
	{
		delete pBinReader;
		std::cout << " . . . FAILED!\n[ERROR] " << "SpriteFont::Load > Failed to read the assetFile!\nPath: " << assetFile << std::endl;

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
	if (!valid) { std::cout << " . . . FAILED!\n[ERROR] " << "Font file header invalid!" << std::endl; return nullptr; }
	if (pBinReader->Read<char>() < 3)
	{
		std::cout << " . . . FAILED!\n[ERROR] " << "Font version invalid!" << std::endl; return nullptr;
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
	if (pagecount > 1)std::cout << "[ERROR] " <<
		"SpriteFont::Load > SpriteFont(.fnt): Only one texture per font allowed" << std::endl;
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
	if (pn.size() == 0)std::cout << "[ERROR] " <<
		"SpriteFont::Load > SpriteFont(.fnt): Invalid Font Sprite [Empty]" << std::endl;
	auto filepath = assetFile.substr(0, assetFile.rfind('/') + 1);
	std::cout << std::endl << "\t";

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
			std::cout << "[WARNING] " <<
				"SpriteFont::Load > SpriteFont(.fnt): Invalid Character" << std::endl;
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

	cout << "  . . . SUCCESS!" << endl;

	return pFont;
}

void FontLoader::Destroy(SpriteFont* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}