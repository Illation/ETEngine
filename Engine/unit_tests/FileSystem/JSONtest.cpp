#include <EtFramework/stdafx.h>
#include <catch2/catch.hpp>

#include <mainTesting.h>

#include <EtMath/MathUtil.h>

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/JsonParser.h>


using namespace et;


TEST_CASE("Parse", "[json]")
{
	core::File* jsonFile = new core::File(global::g_UnitTestDir + "FileSystem/json_test_file.json", nullptr);
	bool openResult = jsonFile->Open( core::FILE_ACCESS_MODE::Read );
	REQUIRE( openResult == true );
	core::JSON::Parser parser = core::JSON::Parser(core::FileUtil::AsText(jsonFile->Read()));
	delete jsonFile;
	jsonFile = nullptr;
	core::JSON::Object* root = parser.GetRoot();
	REQUIRE_FALSE(root == nullptr);
	core::JSON::Bool* boolvalue = (*root)["boolvalue"]->b();
	REQUIRE_FALSE(boolvalue == nullptr);
	REQUIRE(boolvalue->value == true);

	REQUIRE((*root)["falsevalue"]->b()->value == false);

	REQUIRE((*root)["nullvalue"]->GetType() == core::JSON::JSON_Null);

	REQUIRE((*(*root)["menu"]->obj())["id"]->str()->value == "File");

	core::JSON::Array* jnumbers = (*(*root)["menu"]->obj())["num \"array"]->arr();
	REQUIRE((*jnumbers)[0]->GetType() == core::JSON::JSON_Number);
	std::vector<double> numbers = jnumbers->NumArr();
	REQUIRE(math::nearEquals(numbers[0], 10.0) == true);
	REQUIRE(math::nearEquals(numbers[1], 3.345) == true);
	REQUIRE(math::nearEquals(numbers[2], 2.0e4) == true);
	REQUIRE(math::nearEquals(numbers[3], 4.53e-2) == true);
	REQUIRE(math::nearEquals(numbers[4], 0.4e12) == true);
	REQUIRE(math::nearEquals(numbers[5], -12.43e+3) == true);

	core::JSON::Array* menu = (*(*(*root)["menu"]->obj())["popup"]->obj())["menuitem"]->arr();
	REQUIRE((*menu)[0]->GetType() == core::JSON::JSON_Object);
	REQUIRE((*(*menu)[0]->obj())["onclick"]->str()->value == "CreateNewDoc()");
}
