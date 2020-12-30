#include <catch2/catch.hpp>
#include <EtFramework/stdafx.h>

#include <mainTesting.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>
#include <EtCore/IO/JsonParser.h>

#include <EtRendering/SceneStructure/GLTF.h>


using namespace et;
using namespace et::render;


TEST_CASE("Decode Base64", "[gltf]")
{
	std::string base64 = "VGVzdFRleHQ=";
	std::string expected = "TestText";
	std::vector<uint8> decoded;
	REQUIRE(glTF::DecodeBase64(base64, decoded) == true);
	REQUIRE(expected == core::FileUtil::AsText(decoded));
}

std::string fileName = "Box.gltf";
std::string glbFileName = "Corset.glb";

TEST_CASE("Evaluate URI", "[gltf]")
{
	std::string ext;

	std::string baseDir = global::g_UnitTestDir + "Helper/";

	glTF::URI embedded;
	embedded.path = "data:application/octet-stream;base64,AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAvwAAAL8AAAA/AAAAPwAAAL8AAAA/AAAAvwAAAD8AAAA/AAAAPwAAAD8AAAA/AAAAPwAAAL8AAAA/AAAAvwAAAL8AAAA/AAAAPwAAAL8AAAC/AAAAvwAAAL8AAAC/AAAAPwAAAD8AAAA/AAAAPwAAAL8AAAA/AAAAPwAAAD8AAAC/AAAAPwAAAL8AAAC/AAAAvwAAAD8AAAA/AAAAPwAAAD8AAAA/AAAAvwAAAD8AAAC/AAAAPwAAAD8AAAC/AAAAvwAAAL8AAAA/AAAAvwAAAD8AAAA/AAAAvwAAAL8AAAC/AAAAvwAAAD8AAAC/AAAAvwAAAL8AAAC/AAAAvwAAAD8AAAC/AAAAPwAAAL8AAAC/AAAAPwAAAD8AAAC/AAABAAIAAwACAAEABAAFAAYABwAGAAUACAAJAAoACwAKAAkADAANAA4ADwAOAA0AEAARABIAEwASABEAFAAVABYAFwAWABUA";
	REQUIRE(glTF::EvaluateURI(embedded, baseDir) == true);
	REQUIRE(embedded.type == glTF::URI::URI_DATA);

	glTF::URI binary;
	binary.path = "Box0.bin";
	REQUIRE(glTF::EvaluateURI(binary, baseDir) == true);
	REQUIRE(binary.type == glTF::URI::URI_FILE);

	bool is_equal = false;
	if (embedded.binData.size() < binary.binData.size())
		is_equal = std::equal(embedded.binData.begin(), embedded.binData.end(), binary.binData.begin());
	else
		is_equal = std::equal(binary.binData.begin(), binary.binData.end(), embedded.binData.begin());

	REQUIRE(is_equal == true);
	REQUIRE(embedded.binData.size() == binary.binData.size());
}

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

	glTF::Dom dom;
	REQUIRE(glTF::ParseGlTFJson(root, dom) == true);
}

TEST_CASE("Parse GLB asset", "[gltf]")
{
	std::string baseDir = global::g_UnitTestDir + "Helper/";

	core::File* input = new core::File(baseDir + glbFileName, nullptr);
	REQUIRE(input->Open(core::FILE_ACCESS_MODE::Read) == true);
	std::vector<uint8> binaryContent = input->Read();
	REQUIRE(binaryContent.size() > 0);

	glTF::glTFAsset asset;
	REQUIRE(glTF::ParseGLTFData(binaryContent, input->GetPath(), input->GetExtension(), asset) == true);
	delete input;
	input = nullptr;
}