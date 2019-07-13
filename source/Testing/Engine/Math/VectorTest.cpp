#include <catch.hpp>

#include <EtMath/Vector.h>
#include <Engine/Helper/AtomicTypes.h>

TEST_CASE("Generic vector functionality", "[vector]")
{
	int32 input1 = 1;
	int32 input2 = 2;
	int32 input3 = 3;
	int32 input4 = 4;
	int32 input5 = 5;

	SECTION("default constructor")
	{
		etm::vector<5, int32> vec = etm::vector<5, int32>();
		REQUIRE(vec[0] == 0);
		REQUIRE(vec[1] == 0);
		REQUIRE(vec[2] == 0);
		REQUIRE(vec[3] == 0);
		REQUIRE(vec[4] == 0);
	}
	SECTION("single input constructor")
	{
		etm::vector<5, int32> vec = etm::vector<5, int32>(input1);
		REQUIRE(vec[0] == input1);
		REQUIRE(vec[1] == input1);
		REQUIRE(vec[2] == input1);
		REQUIRE(vec[3] == input1);
		REQUIRE(vec[4] == input1);
	}
	SECTION("initializer list constructor")
	{
		etm::vector<5, int32> vec = { input1, input2, input3, input4, input5 };

		SECTION("verify initializer list")
		{
			REQUIRE(vec[0] == input1);
			REQUIRE(vec[1] == input2);
			REQUIRE(vec[2] == input3);
			REQUIRE(vec[3] == input4);
			REQUIRE(vec[4] == input5);
		}
		SECTION("copy constructor")
		{
			etm::vector<5, int32> vec2 = etm::vector<5, int32>(vec);
			REQUIRE(vec2[0] == input1);
			REQUIRE(vec2[1] == input2);
			REQUIRE(vec2[2] == input3);
			REQUIRE(vec2[3] == input4);
			REQUIRE(vec2[4] == input5);
		}
		SECTION("negate operator")
		{
			etm::vector<5, int32> vec2 = -vec;

			//original unharmed
			REQUIRE(vec[0] == input1);
			REQUIRE(vec[1] == input2);
			REQUIRE(vec[2] == input3);
			REQUIRE(vec[3] == input4);
			REQUIRE(vec[4] == input5);
			//new vector inverted
			REQUIRE(vec2[0] == -input1);
			REQUIRE(vec2[1] == -input2);
			REQUIRE(vec2[2] == -input3);
			REQUIRE(vec2[3] == -input4);
			REQUIRE(vec2[4] == -input5);
		}
	}
}

TEST_CASE("specific vec2 functionality", "[vector]")
{
	float input1 = 3.22345f;
	float input2 = 3.345f;

	SECTION("default constructor")
	{
		vec2 vec = vec2();
		REQUIRE(vec.x == 0);
		REQUIRE(vec.y == 0);
	}
	SECTION("single input constructor")
	{
		vec2 vec = vec2(input1);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input1);
	}
	SECTION("argument list constructor")
	{
		vec2 vec = { input1, input2};
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
	}
	SECTION("float arguments constructor")
	{
		vec2 vec = vec2(input1, input2);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
	}
	SECTION("copy constructor")
	{
		vec2 vecOriginal = vec2(input1, input2);
		SECTION("copy constructor")
		{
			vec2 vec = vec2(vecOriginal);
			REQUIRE(vec.x == input1);
			REQUIRE(vec.y == input2);
		}
		SECTION("[] operator")
		{
			vec2 vec = vec2(vecOriginal);
			REQUIRE(vec[0] == input1);
			REQUIRE(vec[1] == input2);
		}
		SECTION("- operator")
		{
			vec2 vec = vec2(-vecOriginal);
			REQUIRE(vecOriginal.x == input1);
			REQUIRE(vecOriginal.y == input2);
			REQUIRE(vec.x == -input1);
			REQUIRE(vec.y == -input2);
		}
	}
	SECTION("perpendicular operation")
	{
		vec2 vec;
		SECTION("with 0")
		{
			vec = vec2(0);
			REQUIRE(etm::nearEqualsV(etm::perpendicular(vec), vec2(0)) == true);
		}
		SECTION("with pos pos")
		{
			vec = vec2(1, 2);
			REQUIRE(etm::nearEqualsV(etm::perpendicular(vec), vec2(-2, 1)) == true );
		}
		SECTION("with pos neg")
		{
			vec = vec2(1, -2);
			REQUIRE(etm::nearEqualsV(etm::perpendicular(vec), vec2(2, 1)) == true );
		}
		SECTION("with neg pos")
		{
			vec = vec2(-1, 2);
			REQUIRE(etm::nearEqualsV(etm::perpendicular(vec), vec2(-2, -1)) == true );
		}
		SECTION("with neg neg")
		{
			vec = vec2(-1, -2);
			REQUIRE(etm::nearEqualsV(etm::perpendicular(vec), vec2(2, -1)) == true );
		}
		REQUIRE(etm::dot(vec, etm::perpendicular(vec)) == 0);
	}
	SECTION("signed angle")
	{
		vec2 a = vec2(3, -1);
		SECTION("with 0")
		{
			a = vec2(0);
			REQUIRE(etm::nearEquals(etm::angleSigned(a, a), 0.f) == true);
		}
		SECTION("with same")
		{
			REQUIRE(etm::nearEquals(etm::angleSigned(a, a), 0.f) == true);
		}
		SECTION("with perpendicular")
		{
			vec2 b = etm::perpendicular(a);
			REQUIRE(etm::nearEquals(etm::angleSigned(a, b), etm::PI_DIV2, 1.000001f) == true);
		}
		SECTION("with negative sign")
		{
			vec2 b = -etm::perpendicular(a);
			REQUIRE(etm::nearEquals(etm::angleSigned(a, b), -etm::PI_DIV2) == true);
		}
		SECTION("with opposite")
		{
			vec2 b = -a;
			REQUIRE(etm::nearEquals(etm::angleSigned(a, b), etm::PI) == true);
		}
		SECTION("45 degrees")
		{
			a = vec2(1, 0);
			vec2 b = vec2(10, 10);
			REQUIRE(etm::nearEquals(etm::angleSigned(a, b), etm::PI*0.25f, 1.000001f) == true);
		}
	}
}

TEST_CASE("specific vec3 functionality", "[vector]") 
{
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
		vec3 vecOriginal = vec3(input1, input2, input3);
		SECTION("copy constructor")
		{
			vec3 vec = vec3(vecOriginal);
			REQUIRE(vec.x == input1);
			REQUIRE(vec.y == input2);
			REQUIRE(vec.z == input3);
		}
		SECTION("[] operator")
		{
			vec3 vec = vec3(vecOriginal);
			REQUIRE(vec[0] == input1);
			REQUIRE(vec[1] == input2);
			REQUIRE(vec[2] == input3);
		}
		SECTION("- operator")
		{
			vec3 vec = vec3(-vecOriginal);
			REQUIRE(vecOriginal.x == input1);
			REQUIRE(vecOriginal.y == input2);
			REQUIRE(vecOriginal.z == input3);
			REQUIRE(vec.x == -input1);
			REQUIRE(vec.y == -input2);
			REQUIRE(vec.z == -input3);
		}
	}
	SECTION("vec2 swizzle")
	{
		vec3 vec = vec3(input1, input2, input3);
		vec2 xy = vec.xy;
		REQUIRE(xy.x == vec.x);
		REQUIRE(xy.y == vec.y);
		vec2 yz = vec.yz;
		REQUIRE(yz.x == vec.y);
		REQUIRE(yz.y == vec.z);
	}
	SECTION("cross product")
	{
		vec3 vec = vec3(input1, input2, input3);
		vec3 bVec;
		SECTION("with 0")
		{
			bVec = vec3(0);
			vec3 result = etm::cross(bVec, bVec);
			REQUIRE(etm::nearEqualsV(result, vec3::ZERO));
		}
		SECTION("with up")
		{
			bVec = vec3::UP;
		}
		SECTION("with down")
		{
			bVec = vec3::DOWN;
		}
		SECTION("with left")
		{
			bVec = vec3::LEFT;
		}
		SECTION("with right")
		{
			bVec = vec3::RIGHT;
			vec3 cVec = vec3::UP;
			vec3 result = etm::cross(bVec, cVec);
			REQUIRE(etm::nearEqualsV(result, vec3::FORWARD));
			result = etm::cross(cVec, bVec);
			REQUIRE(etm::nearEqualsV(result, vec3::BACK));
		}
		SECTION("with forward")
		{
			bVec = vec3::FORWARD;
			vec3 cVec = vec3::RIGHT;
			vec3 result = etm::cross(cVec, bVec);
			REQUIRE(etm::nearEqualsV(result, vec3::DOWN));
			result = etm::cross(bVec, cVec);
			REQUIRE(etm::nearEqualsV(result, vec3::UP));
			cVec = vec3::RIGHT * 2.f;
			result = etm::cross(bVec, cVec);
			REQUIRE(etm::nearEqualsV(result, vec3::UP * 2.f));
			cVec = vec3::UP;
			result = etm::cross(bVec, cVec);
			REQUIRE(etm::nearEqualsV(result, vec3::LEFT));
			result = etm::cross(cVec, bVec);
			REQUIRE(etm::nearEqualsV(result, vec3::RIGHT));
		}
		SECTION("with back")
		{
			bVec = vec3::BACK;
		}
		vec3 result = etm::cross(vec, bVec);
		REQUIRE(etm::dot(vec, result) == 0);
		REQUIRE(etm::dot(bVec, result) == 0);
	}
	SECTION("angle with axis")
	{
		//This also implicitly tests generic angle calculations
		vec3 a = vec3::RIGHT;
		vec3 b;
		vec3 axis;
		float angle;
		SECTION("with 0")
		{
			b = vec3::ZERO;
			angle = etm::angleSafeAxis(a, b, axis);
			REQUIRE(std::isnan(angle));
			REQUIRE(std::isnan(axis.x));
			REQUIRE(std::isnan(axis.y));
			REQUIRE(std::isnan(axis.z));
		}
		SECTION("positive 90 deg")
		{
			b = vec3::UP;
			angle = etm::angleSafeAxis(a, b, axis);
			REQUIRE(angle == etm::PI_DIV2);
			REQUIRE(etm::nearEqualsV(axis, vec3::FORWARD));
		}
		SECTION("negative 90 deg")
		{
			b = vec3::DOWN;
			angle = etm::angleSafeAxis(a, b, axis);
			REQUIRE(angle == etm::PI_DIV2);
			REQUIRE(etm::nearEqualsV(axis, vec3::BACK));
		}
		SECTION("positive 45 deg")
		{
			b = vec3(1, 1, 0);
			angle = etm::angleSafeAxis(a, b, axis);
			REQUIRE(angle == etm::PI_DIV4);
			REQUIRE(etm::nearEqualsV(axis, vec3::FORWARD));
		}
	}
}


TEST_CASE("specific vec4 functionality", "[vector]")
{
	float input1 = 3.22345f;
	float input2 = 3.345f;
	float input3 = 4.f;
	float input4 = 2.f;

	SECTION("default constructor")
	{
		vec4 vec = vec4();
		REQUIRE(vec.x == 0);
		REQUIRE(vec.y == 0);
		REQUIRE(vec.z == 0);
		REQUIRE(vec.w == 0);
	}
	SECTION("single input constructor")
	{
		vec4 vec = vec4(input1);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input1);
		REQUIRE(vec.z == input1);
		REQUIRE(vec.w == input1);
	}
	SECTION("argument list constructor")
	{
		vec4 vec = { input1, input2, input3, input4 };
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
		REQUIRE(vec.w == input4);

		REQUIRE(vec.r == input1);
		REQUIRE(vec.g == input2);
		REQUIRE(vec.b == input3);
		REQUIRE(vec.a == input4);

		REQUIRE(vec[0] == input1);
		REQUIRE(vec[1] == input2);
		REQUIRE(vec[2] == input3);
		REQUIRE(vec[3] == input4);
	}
	SECTION("float arguments constructor")
	{
		vec4 vec = vec4(input1, input2, input3, input4);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
		REQUIRE(vec.w == input4);
	}
	SECTION("vec2 - vec2 arguments constructor")
	{
		vec4 vec = vec4(vec2(input1, input2), vec2(input3, input4));
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
		REQUIRE(vec.w == input4);
	}
	SECTION("vec2 - float - float arguments constructor")
	{
		vec4 vec = vec4(vec2(input1, input2), input3, input4);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
		REQUIRE(vec.w == input4);
	}
	SECTION("float - float - vec2 arguments constructor")
	{
		vec4 vec = vec4(input1, input2, vec2(input3, input4));
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
		REQUIRE(vec.w == input4);
	}
	SECTION("float - vec2 - float arguments constructor")
	{
		vec4 vec = vec4(input1, vec2(input2, input3), input4);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
		REQUIRE(vec.w == input4);
	}
	SECTION("float - vec3 arguments constructor")
	{
		vec4 vec = vec4(input1, vec3(input2, input3, input4));
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
		REQUIRE(vec.w == input4);
	}
	SECTION("vec3 - float arguments constructor")
	{
		vec4 vec = vec4(vec3(input1, input2, input3), input4);
		REQUIRE(vec.x == input1);
		REQUIRE(vec.y == input2);
		REQUIRE(vec.z == input3);
		REQUIRE(vec.w == input4);
	}
	SECTION("copy constructor")
	{
		vec4 vecOriginal = vec4(input1, input2, input3, input4);
		SECTION("copy constructor")
		{
			vec4 vec = vec4(vecOriginal);
			REQUIRE(vec.x == input1);
			REQUIRE(vec.y == input2);
			REQUIRE(vec.z == input3);
			REQUIRE(vec.w == input4);
		}
		SECTION("- operator")
		{
			vec4 vec = vec4(-vecOriginal);
			REQUIRE(vecOriginal.x == input1);
			REQUIRE(vecOriginal.y == input2);
			REQUIRE(vecOriginal.z == input3);
			REQUIRE(vecOriginal.w == input4);
			REQUIRE(vec.x == -input1);
			REQUIRE(vec.y == -input2);
			REQUIRE(vec.z == -input3);
			REQUIRE(vec.w == -input4);
		}
	}
	SECTION("vec2 swizzle")
	{
		vec4 vec = vec4(input1, input2, input3, input4);
		vec2 xy = vec.xy;
		REQUIRE(xy.x == vec.x);
		REQUIRE(xy.y == vec.y);
		vec2 yz = vec.yz;
		REQUIRE(yz.x == vec.y);
		REQUIRE(yz.y == vec.z);
		vec2 zw = vec.zw;
		REQUIRE(zw.x == vec.z);
		REQUIRE(zw.y == vec.w);
	}
	SECTION("vec3 swizzle")
	{
		vec4 vec = vec4(input1, input2, input3, input4);
		vec3 xyz = vec.xyz;
		REQUIRE(xyz.x == vec.x);
		REQUIRE(xyz.y == vec.y);
		REQUIRE(xyz.z == vec.z);
		vec3 rgb = vec.rgb;
		REQUIRE(rgb.x == vec.x);
		REQUIRE(rgb.y == vec.y);
		REQUIRE(rgb.z == vec.z);
		REQUIRE(rgb.x == vec.r);
		REQUIRE(rgb.y == vec.g);
		REQUIRE(rgb.z == vec.b);
	}
}

TEST_CASE("vector math", "[vector]")
{
	float input1 = 2.5f;
	float input2 = 3.3f;
	float input3 = 4.f;

	vec3 vecA = vec3(input1, input2, input3);
	vec3 vecB = vec3(input3, input2, input1);
	vec3 vecC = vec3(-input2, input3, -input1);

	SECTION("add")
	{
		REQUIRE(etm::nearEqualsV(vecA+vecB, vec3(6.5f, 6.6f, 6.5f)));
		REQUIRE(etm::nearEqualsV(vecA+vecC, vec3(-0.8f, 7.3f, 1.5f), 0.000001f));
		REQUIRE(etm::nearEqualsV(vecB+vecC, vec3(0.7f, 7.3f, 0.f), 0.000001f));
	}
	SECTION("sub")
	{
		REQUIRE(etm::nearEqualsV(vecA-vecB, vec3(-1.5f, 0.f, 1.5f)));
		REQUIRE(etm::nearEqualsV(vecA-vecC, vec3(5.8f, -0.7f, 6.5f), 0.000001f));
		REQUIRE(etm::nearEqualsV(vecB-vecC, vec3(7.3f, -0.7f, 5.f), 0.000001f));
	}
	SECTION("mul")
	{
		REQUIRE(etm::nearEqualsV(vecA*vecB, vec3(10.f, 10.89f, 10.f), 0.000001f));
		REQUIRE(etm::nearEqualsV(vecA*vecC, vec3(-8.25f, 13.2f, -10.f), 0.000001f));
		REQUIRE(etm::nearEqualsV(vecB*vecC, vec3(-13.2f, 13.2f, -6.25f), 0.000001f));
	}
	SECTION("div")
	{
		REQUIRE(etm::nearEqualsV(vecA/vecB, vec3(0.625f, 1.f, 1.6f)));
		REQUIRE(etm::nearEqualsV(vecA/vecC, vec3(-0.7575757575757576f, 0.825f, -1.6f)));
		REQUIRE(etm::nearEqualsV(vecB/vecC, vec3(-1.212121212121212f, 0.825f, -1.f)));
	}
	SECTION("dot")
	{
		REQUIRE(etm::nearEquals(etm::dot(vecA,vecB), 30.89f));
		REQUIRE(etm::nearEquals(etm::dot(vecA,vecC), -5.05f));
		REQUIRE(etm::nearEquals(etm::dot(vecB,vecC), -6.25f));
	}
	SECTION("dist")//by verifying this works, length and lengthSq must also work
	{
		REQUIRE(etm::nearEquals(etm::distance(vecA,vecB), 2.12132f, 0.00001f));
		REQUIRE(etm::nearEquals(etm::distance(vecA,vecC), 8.739565206576355f, 0.000001f));
		REQUIRE(etm::nearEquals(etm::distance(vecB,vecC), 8.875809822207774f, 0.000001f));
	}
	SECTION("normalize")
	{
		REQUIRE(etm::nearEqualsV(etm::normalize(vecA), vec3(0.4342739277195007f, 0.5732415845897409f, 0.6948382843512011f)));
		REQUIRE(etm::nearEquals(etm::length(etm::normalize(vecA)), 1.f));
		REQUIRE(etm::nearEqualsV(etm::normalize(vecB), vec3(0.6948382843512011f, 0.5732415845897409f, 0.4342739277195007f)));
		REQUIRE(etm::nearEquals(etm::length(etm::normalize(vecB)), 1.f));
		REQUIRE(etm::nearEqualsV(etm::normalize(vecC), vec3(-0.5732415845897409f, 0.6948382843512011f, -0.4342739277195007f)));
		REQUIRE(etm::nearEquals(etm::length(etm::normalize(vecC)), 1.f));
	}
	//angles already tested by extension of specific vec 3 solution
}