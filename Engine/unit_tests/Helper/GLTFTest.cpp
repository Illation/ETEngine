#include <catch2/catch.hpp>
#include <EtFramework/stdafx.h>

#include <mainTesting.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>
#include <EtCore/IO/JsonParser.h>

#include <EtEditor/Import/GltfParser.h>


using namespace et;


std::string const fileName = "Box.gltf";
std::string const glbFileName = "Corset.glb";


TEST_CASE("Parse GLTF json", "[gltf]")
{
	std::string baseDir = global::g_UnitTestDir + "Helper/";

	core::File* input = new core::File(baseDir+fileName, nullptr);
	REQUIRE(input->Open(core::FILE_ACCESS_MODE::Read) == true);
	std::vector<uint8> binaryContent = input->Read();
	std::string extension = input->GetExtension();
	delete input;
	input = nullptr;
	REQUIRE(binaryContent.size() > 0);
	core::JSON::Parser parser = core::JSON::Parser(core::FileUtil::AsText(binaryContent));
	core::JSON::Object* root = parser.GetRoot();
	REQUIRE(root != nullptr);

	edit::glTF::Dom dom;
	REQUIRE(edit::glTF::ParseGlTFJson(root, dom) == true);
}

TEST_CASE("Parse GLB asset", "[gltf]")
{
	std::string baseDir = global::g_UnitTestDir + "Helper/";

	core::File* input = new core::File(baseDir + glbFileName, nullptr);
	REQUIRE(input->Open(core::FILE_ACCESS_MODE::Read) == true);
	std::vector<uint8> binaryContent = input->Read();
	REQUIRE(binaryContent.size() > 0);

	edit::glTF::glTFAsset asset;
	REQUIRE(edit::glTF::ParseGLTFData(binaryContent, input->GetPath(), input->GetExtension(), asset) == true);
	delete input;
	input = nullptr;
}