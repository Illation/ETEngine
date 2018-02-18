#include "../../../Engine/stdafx.hpp"
#include <catch.hpp>

#include "../../../Engine/FileSystem/Entry.h"
#include "../../../Engine/FileSystem/FileUtil.h"
#include "../../../Engine/FileSystem/JSONparser.h"
#include "../../../Engine/Math/MathUtil.hpp"

TEST_CASE("Parse", "[json]")
{
	File* jsonFile = new File("./source/Testing/Engine/FileSystem/json_test_file.json", nullptr);
	bool openResult = jsonFile->Open( FILE_ACCESS_MODE::Read );
	REQUIRE( openResult == true );
	JSON::Parser parser = JSON::Parser(FileUtil::AsText(jsonFile->Read()));
	delete jsonFile;
	jsonFile = nullptr;
	JSON::Object* root = parser.GetRoot();
	REQUIRE_FALSE(root == nullptr);
	JSON::Bool* boolvalue = (*root)["boolvalue"]->b();
	REQUIRE_FALSE(boolvalue == nullptr);
	REQUIRE(boolvalue->value == true);

	REQUIRE((*root)["falsevalue"]->b()->value == false);

	REQUIRE((*root)["nullvalue"]->GetType() == JSON::JSON_Null);

	REQUIRE((*(*root)["menu"]->obj())["id"]->str()->value == "File");

	JSON::Array* jnumbers = (*(*root)["menu"]->obj())["num \"array"]->arr();
	REQUIRE((*jnumbers)[0]->GetType() == JSON::JSON_Number);
	std::vector<double> numbers = jnumbers->NumArr();
	REQUIRE(etm::nearEquals(numbers[0], 10.0) == true);
	REQUIRE(etm::nearEquals(numbers[1], 3.345) == true);
	REQUIRE(etm::nearEquals(numbers[2], 2.0e4) == true);
	REQUIRE(etm::nearEquals(numbers[3], 4.53e-2) == true);
	REQUIRE(etm::nearEquals(numbers[4], 0.4e12) == true);
	REQUIRE(etm::nearEquals(numbers[5], -12.43e+3) == true);

	JSON::Array* menu = (*(*(*root)["menu"]->obj())["popup"]->obj())["menuitem"]->arr();
	REQUIRE((*menu)[0]->GetType() == JSON::JSON_Object);
	REQUIRE((*(*menu)[0]->obj())["onclick"]->str()->value == "CreateNewDoc()");
}