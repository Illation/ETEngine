#include <catch.hpp>

#include "../../../Engine/FileSystem/Entry.h"
#include "../../../Engine/FileSystem/FileUtil.h"
#include "../../../Engine/FileSystem/JSONparser.h"

TEST_CASE("Parse", "[json]")
{
	File* jsonFile = new File("./source/Testing/Engine/FileSystem/json_test_file.json", nullptr);
	bool openResult = jsonFile->Open( FILE_ACCESS_MODE::Read );
	REQUIRE( openResult == true );
	JSONparser parser = JSONparser(FileUtil::AsText(jsonFile->Read()));
	JSONobject* root = parser.GetRoot();
	REQUIRE_FALSE(root == nullptr);
}