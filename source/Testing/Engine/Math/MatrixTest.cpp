#include <catch.hpp>

#include "../../../Engine/Math/Matrix.h"

TEST_CASE("Generic matrix functionality", "[matrix]")
{
	using namespace etm;

	float in1 = 5.3f;
	float in2 = 1.2f;
	float in3 = 3.0f;
	float in4 = 2.3f;
	float in5 = 5.8f;
	float in6 = 7.3f;
	float in7 = 3.2f;
	float in8 = 4.9f;
	float in9 = 6.3f;

	SECTION("default constructor")
	{
		mat3 mat = mat3();
		REQUIRE(mat[0][0] == 1);
		REQUIRE(mat[0][1] == 0);
		REQUIRE(mat[0][2] == 0);
		REQUIRE(mat[1][0] == 0);
		REQUIRE(mat[1][1] == 1);
		REQUIRE(mat[1][2] == 0);
		REQUIRE(mat[2][0] == 0);
		REQUIRE(mat[2][1] == 0);
		REQUIRE(mat[2][2] == 1);
	}
	SECTION("initializer list constructor")
	{
		mat3 mat = { in1, in2, in3, in4, in5, in6, in7, in8, in9 };
		REQUIRE(mat[0][0] == in1);
		REQUIRE(mat[0][1] == in2);
		REQUIRE(mat[0][2] == in3);
		REQUIRE(mat[1][0] == in4);
		REQUIRE(mat[1][1] == in5);
		REQUIRE(mat[1][2] == in6);
		REQUIRE(mat[2][0] == in7);
		REQUIRE(mat[2][1] == in8);
		REQUIRE(mat[2][2] == in9);
	}
	SECTION("row list constructor")
	{
		mat3 mat(new vec3[3]{ vec3(in1, in2, in3), vec3(in4, in5, in6), vec3(in7, in8, in9) });
		REQUIRE(mat[0][0] == in1);
		REQUIRE(mat[0][1] == in2);
		REQUIRE(mat[0][2] == in3);
		REQUIRE(mat[1][0] == in4);
		REQUIRE(mat[1][1] == in5);
		REQUIRE(mat[1][2] == in6);
		REQUIRE(mat[2][0] == in7);
		REQUIRE(mat[2][1] == in8);
		REQUIRE(mat[2][2] == in9);
	}
	SECTION("row access operator")
	{
		vec3 v1(in1, in2, in3);
		vec3 v2(in4, in5, in6);
		vec3 v3(in7, in8, in9);
		mat3 mat(new vec3[3]{ v1, v2, v3 });
		REQUIRE(nearEqualsV(mat[0], v1));
		REQUIRE(nearEqualsV(mat[1], v2));
		REQUIRE(nearEqualsV(mat[2], v3));
	}
}