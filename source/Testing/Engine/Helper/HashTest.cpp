#include <catch.hpp>
#include "../../../Engine/Helper/Hash.h"


TEST_CASE("String Hash", "[hash]")
{
	constexpr uint32 check = "0123456789ABCDEF"_hash;
	REQUIRE(check == 141695047u);
	REQUIRE(check == FnvHash("0123456789ABCDEF"));
}