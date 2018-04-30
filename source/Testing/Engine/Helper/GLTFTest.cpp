#include <catch.hpp>
#include "../../../Engine/Helper/GLTF.h"
#include "../../../Engine/FileSystem/FileUtil.h"
#include "../../../Engine/FileSystem/Entry.h"
#include "../../../Engine/FileSystem/JSONparser.h"

TEST_CASE("Decode Base64", "[gltf]")
{
	std::string base64 = "VGVzdFRleHQ=";
	std::string expected = "TestText";
	std::vector<uint8> decoded;
	REQUIRE(glTF::DecodeBase64(base64, decoded) == true);
	REQUIRE(expected == FileUtil::AsText(decoded));
}

std::string baseDir = "./source/Testing/Engine/Helper/";
std::string fileName = "Box.gltf";
std::string glbFileName = "Corset.glb";

TEST_CASE("Evaluate URI", "[gltf]")
{
	std::string ext;

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
	File* input = new File(baseDir+fileName, nullptr);
	REQUIRE(input->Open(FILE_ACCESS_MODE::Read) == true);
	std::vector<uint8> binaryContent = input->Read();
	std::string extension = input->GetExtension();
	delete input;
	input = nullptr;
	REQUIRE(binaryContent.size() > 0);
	JSON::Parser parser = JSON::Parser(FileUtil::AsText(binaryContent));
	JSON::Object* root = parser.GetRoot();
	REQUIRE(root != nullptr);

	glTF::Dom dom;
	REQUIRE(glTF::ParseGlTFJson(root, dom) == true);
}

TEST_CASE("Parse GLB asset", "[gltf]")
{
	File* input = new File(baseDir + glbFileName, nullptr);
	REQUIRE(input->Open(FILE_ACCESS_MODE::Read) == true);
	std::vector<uint8> binaryContent = input->Read();
	REQUIRE(binaryContent.size() > 0);

	glTF::glTFAsset asset;
	REQUIRE(glTF::ParseGLTFData(binaryContent, input->GetPath(), input->GetExtension(), asset) == true);
	delete input;
	input = nullptr;
}