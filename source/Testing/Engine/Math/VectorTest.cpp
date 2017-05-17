#include "../../catch.hpp"

#include "../../../Engine/Math/Vector.h"

TEST_CASE("Vector<3> is constructed", "[vector]") 
{
	using namespace etm;

	float input1 = 3.22345f;
	float input2 = 3.345f;
	float input3 = 4.f;

	SECTION("default constructor")
	{
		vec3 vec = vec3();
		REQUIRE(vec.x == 0);
		REQUIRE(vec.y == 0);
		REQUIRE(vec.z == 0);
	}
	SECTION("single input constructor")
	{
		vec3 vec = vec3(input1);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input1);
		REQUIRE(vec.z == input1);
	}
	SECTION("argument list constructor")
	{
		vec3 vec = { input1, input2, input3 };
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
	}
	SECTION("float arguments constructor")
	{
		vec3 vec = vec3(input1, input2, input3);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
	}
	SECTION("vec2 - float arguments constructor")
	{
		vec3 vec = vec3(vec2(input1, input2), input3);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
	}
	SECTION("float - vec2 arguments constructor")
	{
		vec3 vec = vec3(input1, vec2(input2, input3));
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
	}
	SECTION("copy constructor")
	{
		vec3 vec = vec3(vec3(input1, input2, input3));
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
	}
}