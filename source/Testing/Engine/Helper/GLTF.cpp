#include <catch.hpp>
#include "../../../Engine/Helper/GLTF.h"
#include "../../../Engine/FileSystem/FileUtil.h"

TEST_CASE("Decode Base64", "[gltf]")
{
	std::string base64 = "VGVzdFRleHQ=";
	std::string expected = "TestText";
	std::vector<uint8> decoded;
	REQUIRE(glTF::DecodeBase64(base64, decoded) == true);
	REQUIRE(expected == FileUtil::AsText(decoded));
}