#include <catch.hpp>

#include "../../../Engine/Math/Transform.hpp"

TEST_CASE("scale", "[transform]")
{
	using namespace etm;

	SECTION("2D scale")
	{
		vec2 testVec = vec2( 1, 0 );
		mat2 scaleMat = etm::scale( vec2( 3 ) );
		SECTION("no ref function")
		{
			vec2 resultVec = scaleMat * testVec;
			REQUIRE( nearEqualsV( resultVec, vec2( 3, 0 ), 0.0001f ) );
		}
		SECTION("ref function")
		{
			etm::scale( scaleMat, vec2( 0.5 ) );
			vec2 resultVec = scaleMat * testVec;
			REQUIRE( nearEqualsV( resultVec, vec2( 1.5f, 0 ), 0.0001f ) );
		}
	}
	//3d vectors create 4x4 matricies
	SECTION("3D scale")
	{
		vec3 testVec = vec3( 1, 0, 0 );
		mat4 scaleMat = etm::scale( vec3( 3 ) );
		SECTION("no ref function")
		{
			vec3 resultVec = (scaleMat * vec4(testVec, 0)).xyz;
			REQUIRE( nearEqualsV( resultVec, vec3( 3, 0, 0 ), 0.0001f ) );
		}
		SECTION("ref function")
		{
			etm::scale( scaleMat, vec3( 0.5 ) );
			vec3 resultVec = (scaleMat * vec4(testVec, 0)).xyz;
			REQUIRE( nearEqualsV( resultVec, vec3( 1.5f, 0, 0 ), 0.0001f ) );
		}
	}
}

TEST_CASE("rotate", "[transform]")
{
	using namespace etm;

	vec3 testVec = vec3( 1, 0, 0 );
	mat4 rotMat = etm::rotate( vec3( 0, 0, 1 ), etm::PI_DIV2 );
	SECTION("from axis angle")
	{
		vec3 resultVec = (rotMat * vec4(testVec, 0)).xyz;
		REQUIRE( nearEqualsV( resultVec, vec3( 0, 1, 0 ), 0.0001f ) );
	}
	SECTION("ref function")
	{
		etm::rotate( rotMat, vec3( 0, 0, 1 ), -etm::PI_DIV2 );
		vec3 resultVec = (rotMat * vec4(testVec, 0)).xyz;
		REQUIRE( nearEqualsV( resultVec, testVec, 0.0001f ) );
	}
	SECTION( "quaternions" )
	{
		quat rotQuat = quat( vec3( 0, 0, 1 ), etm::PI_DIV2 );
		SECTION("quaterion rotation compatibility")
		{
			vec3 resultVec = (rotMat * vec4(testVec, 0)).xyz;
			vec3 quatResultVec = rotQuat * testVec;

			REQUIRE( nearEqualsV( resultVec, quatResultVec, 0.0001f ) );
		}
		SECTION("from quaterion")
		{
			mat4 quatRotMat = etm::rotate( rotQuat );
			REQUIRE( nearEqualsM( quatRotMat, rotMat, 0.0001f ) );

			REQUIRE( nearEqualsM( rotQuat.ToMatrix(), CreateFromMat4( quatRotMat ), 0.0001f ) );

			vec3 resultVec = (quatRotMat * vec4(testVec, 0)).xyz;
			REQUIRE( nearEqualsV( resultVec, vec3( 0, 1, 0 ), 0.0001f ) );
		}
		SECTION("ref function")
		{
			mat4 quatRotMat = etm::rotate( rotQuat );
			etm::rotate( quatRotMat, etm::inverse( rotQuat ) );
			vec3 resultVec = (quatRotMat * vec4(testVec, 0)).xyz;
			REQUIRE( nearEqualsV( resultVec, testVec, 0.0001f ) );
		}
	}
}

TEST_CASE( "translate", "[transform]" )
{
	using namespace etm;

	vec3 testVec = vec3( 1, 0, 0 );
	mat4 moveMat = etm::translate( vec3( 0, 0, 1 ) );
	SECTION( "no ref" )
	{
		vec3 resultVec = (moveMat * vec4( testVec, 1 )).xyz;
		REQUIRE( nearEqualsV( resultVec, vec3( 1, 0, 1 ), 0.0001f ) );
	}
	SECTION( "with ref" )
	{
		etm::translate( moveMat, vec3( 0, 0, -1 ) );
		vec3 resultVec = (moveMat * vec4( testVec, 0 )).xyz;
		REQUIRE( nearEqualsV( resultVec, testVec, 0.0001f ) );
	}
}

TEST_CASE( "complete transform", "[transform]" )
{
	using namespace etm;

	vec3 testVec = vec3( 0, 1, 0 );

	mat4 scaleMat = etm::scale( vec3( 3 ) );
	mat4 rotMat = etm::rotate( vec3( 0, 0, 1 ), -etm::PI_DIV2 );
	mat4 moveMat = etm::translate( vec3( 0, 0, 1 ) ); // #todo matricies should multiply with vectors in column major notation to make memory of transformation matrices aligned - right now we would need to transpose the matrix to achieve the same transformation in opengl
	mat4 transformMat = moveMat * rotMat * scaleMat; // #todo matrices should do multiplication the other way around to make this readable in the correct order

	vec3 resultVec = (transformMat * vec4( testVec, 1 )).xyz;

	REQUIRE( nearEqualsV( resultVec, vec3(3, 0, 1), 0.00001f ) );
}
