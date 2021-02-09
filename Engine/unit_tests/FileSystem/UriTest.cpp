#include <catch2/catch.hpp>
#include <EtFramework/stdafx.h>

#include <mainTesting.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/Uri.h>


using namespace et;


TEST_CASE("Decode Base64", "[uri]")
{
	std::string base64 = "VGVzdFRleHQ=";
	std::string expected = "TestText";
	std::vector<uint8> decoded;
	REQUIRE(core::URI::DecodeBase64(base64, decoded) == true);
	REQUIRE(expected == core::FileUtil::AsText(decoded));
}

TEST_CASE("Evaluate URI", "[uri]")
{
	std::string ext;

	std::string baseDir = global::g_UnitTestDir + "Helper/";

	core::URI embedded;
	embedded.SetPath("data:application/octet-stream;base64,AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAAAAAAIA/AAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAAAAAAAAgL8AAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAAAAAAAAgD8AAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAACAvwAAAAAAAAAAAAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAAAAAAAAAAIC/AAAAvwAAAL8AAAA/AAAAPwAAAL8AAAA/AAAAvwAAAD8AAAA/AAAAPwAAAD8AAAA/AAAAPwAAAL8AAAA/AAAAvwAAAL8AAAA/AAAAPwAAAL8AAAC/AAAAvwAAAL8AAAC/AAAAPwAAAD8AAAA/AAAAPwAAAL8AAAA/AAAAPwAAAD8AAAC/AAAAPwAAAL8AAAC/AAAAvwAAAD8AAAA/AAAAPwAAAD8AAAA/AAAAvwAAAD8AAAC/AAAAPwAAAD8AAAC/AAAAvwAAAL8AAAA/AAAAvwAAAD8AAAA/AAAAvwAAAL8AAAC/AAAAvwAAAD8AAAC/AAAAvwAAAL8AAAC/AAAAvwAAAD8AAAC/AAAAPwAAAL8AAAC/AAAAPwAAAD8AAAC/AAABAAIAAwACAAEABAAFAAYABwAGAAUACAAJAAoACwAKAAkADAANAA4ADwAOAA0AEAARABIAEwASABEAFAAVABYAFwAWABUA");
	REQUIRE(embedded.Evaluate(baseDir) == true);
	REQUIRE(embedded.GetType() == core::URI::E_Type::Data);

	core::URI binary;
	binary.SetPath("Box0.bin");
	REQUIRE(binary.Evaluate(baseDir) == true);
	REQUIRE(binary.GetType() == core::URI::E_Type::FileRelative);

	bool is_equal = false;
	if (embedded.GetEvaluatedData().size() < binary.GetEvaluatedData().size())
	{
		is_equal = std::equal(embedded.GetEvaluatedData().begin(), embedded.GetEvaluatedData().end(), binary.GetEvaluatedData().begin());
	}
	else
	{
		is_equal = std::equal(binary.GetEvaluatedData().begin(), binary.GetEvaluatedData().end(), embedded.GetEvaluatedData().begin());
	}

	REQUIRE(is_equal == true);
	REQUIRE(embedded.GetEvaluatedData().size() == binary.GetEvaluatedData().size());
}
