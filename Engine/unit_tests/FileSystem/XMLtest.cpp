#include <EtFramework/stdafx.h>
#include <catch2/catch.hpp>

#include <mainTesting.h>

#include <EtMath/MathUtil.h>

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/XmlParser.h>


using namespace et;


TEST_CASE("ParseXML", "[xml]")
{
	core::File* xmlFile = new core::File(global::g_UnitTestDir + "FileSystem/xml_test_file.xml", nullptr);
	bool openResult = xmlFile->Open(core::FILE_ACCESS_MODE::Read);
	REQUIRE(openResult == true);
	core::XML::Parser parser = core::XML::Parser(core::FileUtil::AsText(xmlFile->Read()));
	delete xmlFile;
	xmlFile = nullptr;
}
