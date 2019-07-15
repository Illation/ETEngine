#include <catch.hpp>
#include <EtCore/Helper/Hash.h>


TEST_CASE("String Hash", "[hash]")
{
	constexpr T_Hash check = "0123456789ABCDEF"_hash;
	REQUIRE(check == 141695047u);
	REQUIRE(check == GetHash("0123456789ABCDEF"));
}