#include "stdafx.hpp"
#include "StarField.h"
#include "FileSystem\Entry.h"
#include "FileSystem\JSONparser.h"
#include "FileSystem\FileUtil.h"

StarField::StarField(const std::string &dataFile) :
	m_DataFile(dataFile)
{ }

void StarField::Initialize()
{
	File* jsonFile = new File(m_DataFile, nullptr);
	if (!jsonFile->Open(FILE_ACCESS_MODE::Read))
		return;

	JSON::Parser parser = JSON::Parser(FileUtil::AsText(jsonFile->Read()));
	delete jsonFile;
	jsonFile = nullptr;

	JSON::Object* root = parser.GetRoot();

	JSON::Array* jstarArray = (*root)["stars"]->arr();
	for (auto jStar : jstarArray->value)
	{
		vec4 star;
		if (JSON::ArrayVector(jStar, star))
			m_Stars.push_back(star);
	}
}

void StarField::Update()
{

}

void StarField::DrawForward()
{

}
