#include "../../catch.hpp"

#include "../../../Engine/Math/Vector.h"

TEST_CASE("Generic vector functionality", "[vector]")
{
	using namespace etm;

	int32 input1 = 1;
	int32 input2 = 2;
	int32 input3 = 3;
	int32 input4 = 4;
	int32 input5 = 5;

	SECTION("default constructor")
	{
		vector<5, int32> vec = vector<5, int32>();
		REQUIRE(vec[0] == 0);
		REQUIRE(vec[1] == 0);
		REQUIRE(vec[2] == 0);
		REQUIRE(vec[3] == 0);
		REQUIRE(vec[4] == 0);
	}
	SECTION("single input constructor")
	{
		vector<5, int32> vec = vector<5, int32>(input1);
		REQUIRE(vec[0] == input1);
		REQUIRE(vec[1] == input1);
		REQUIRE(vec[2] == input1);
		REQUIRE(vec[3] == input1);
		REQUIRE(vec[4] == input1);
	}
	//SECTION("initializer list constructor")
	//{
	//	vector<5, int32> vec = vector<5, int32>({ input1, input2, input3, input4, input5 });

	//	SECTION("verify initializer list")
	//	{
	//		REQUIRE(vec[0] == input1);
	//		REQUIRE(vec[1] == input2);
	//		REQUIRE(vec[2] == input3);
	//		REQUIRE(vec[3] == input4);
	//		REQUIRE(vec[4] == input5);
	//	}
	//	SECTION("copy constructor")
	//	{
	//		vector<5, int32> vec2 = vector<5, int32>(vec);
	//		REQUIRE(vec2[0] == input1);
	//		REQUIRE(vec2[1] == input2);
	//		REQUIRE(vec2[2] == input3);
	//		REQUIRE(vec2[3] == input4);
	//		REQUIRE(vec2[4] == input5);
	//	}
	//	SECTION("negate operator")
	//	{
	//		vector<5, int32> vec2 = -vec;

	//		//original unharmed
	//		REQUIRE(vec[0] == input1);
	//		REQUIRE(vec[1] == input2);
	//		REQUIRE(vec[2] == input3);
	//		REQUIRE(vec[3] == input4);
	//		REQUIRE(vec[4] == input5);
	//		//new vector inverted
	//		REQUIRE(vec2[0] == -input1);
	//		REQUIRE(vec2[1] == -input2);
	//		REQUIRE(vec2[2] == -input3);
	//		REQUIRE(vec2[3] == -input4);
	//		REQUIRE(vec2[4] == -input5);
	//	}
	//}
}

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