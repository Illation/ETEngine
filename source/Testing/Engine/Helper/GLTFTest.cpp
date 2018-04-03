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

TEST_CASE("Evaluate URI", "[gltf]")
{
	std::string ext;

	std::string embedded = "data:application/octet-stream;base64,AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAvwAAAL8AAAA/AAAAPwAAAL8AAAA/AAAAvwAAAD8AAAA/AAAAPwAAAD8AAAA/AAAAPwAAAL8AAAA/AAAAvwAAAL8AAAA/AAAAPwAAAL8AAAC/AAAAvwAAAL8AAAC/AAAAPwAAAD8AAAA/AAAAPwAAAL8AAAA/AAAAPwAAAD8AAAC/AAAAPwAAAL8AAAC/AAAAvwAAAD8AAAA/AAAAPwAAAD8AAAA/AAAAvwAAAD8AAAC/AAAAPwAAAD8AAAC/AAAAvwAAAL8AAAA/AAAAvwAAAD8AAAA/AAAAvwAAAL8AAAC/AAAAvwAAAD8AAAC/AAAAvwAAAL8AAAC/AAAAvwAAAD8AAAC/AAAAPwAAAL8AAAC/AAAAPwAAAD8AAAC/AAABAAIAAwACAAEABAAFAAYABwAGAAUACAAJAAoACwAKAAkADAANAA4ADwAOAA0AEAARABIAEwASABEAFAAVABYAFwAWABUA";
	std::vector<uint8> decEmbedded;
	REQUIRE(glTF::EvaluateURI(embedded, decEmbedded, ext, baseDir) == true);

	std::string binary = "Box0.bin";
	std::vector<uint8> decBin;
	REQUIRE(glTF::EvaluateURI(binary, decBin, ext, baseDir) == true);

	bool is_equal = false;
	if (decEmbedded.size() < decBin.size())
		is_equal = std::equal(decEmbedded.begin(), decEmbedded.end(), decBin.begin());
	else
		is_equal = std::equal(decBin.begin(), decBin.end(), decEmbedded.begin());

	REQUIRE(is_equal == true);
	REQUIRE(decEmbedded.size() == decBin.size());
}

TEST_CASE("Parse GLTF", "[gltf]")
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