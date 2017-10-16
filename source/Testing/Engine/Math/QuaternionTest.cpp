#include <catch.hpp>

#include "../../../Engine/Math/Quaternion.hpp"

TEST_CASE("constructors", "[quat]")
{
	using namespace etm;

	SECTION( "default constructor" )
	{
		quat testQuat = quat();
		REQUIRE( testQuat.x == 0 );
		REQUIRE( testQuat.y == 0 );
		REQUIRE( testQuat.z == 0 );
		REQUIRE( testQuat.w == 1 );
	}
	SECTION( "axis constructor" )
	{
		quat testQuat = quat(vec3(1, 0, 0));
		REQUIRE( testQuat.x == 1 );
		REQUIRE( testQuat.y == 0 );
		REQUIRE( testQuat.z == 0 );
		REQUIRE( testQuat.w == 0 );
	}
	SECTION( "value constructor" )
	{
		quat testQuat = quat(3, 2, 4, 2.4f);
		REQUIRE( testQuat.x == 3 );
		REQUIRE( testQuat.y == 2 );
		REQUIRE( testQuat.z == 4 );
		REQUIRE( testQuat.w == 2.4f );
	}
}
TEST_CASE("axis angles", "[quat]")
{
	using namespace etm;

	float angle = etm::PI_DIV4;
	quat testQuat = quat( normalize( vec3( -1, -1, 1 ) ), angle );
	SECTION( "from axis angle" )
	{
		REQUIRE( nearEqualsV( testQuat.v4, vec4( -0.2209424f, -0.2209424f, 0.2209424f, 0.9238795f ), 0.0001f ) );
	}
	SECTION( "to axis angle" )
	{
		vec4 axisAngle = testQuat.ToAxisAngle();
		REQUIRE( nearEqualsV( axisAngle, vec4( normalize( vec3( -1, -1, 1 ) ), angle ), 0.0001f ) );
	}
	SECTION( "use axis angle quat for rotation" )
	{
		vec3 initial = vec3( 0, 1, 0 );
		quat testQuat2 = quat( vec3( 1, 0, 0 ), etm::PI_DIV2 );
		vec3 rotated2 = testQuat2 * initial;
		REQUIRE( nearEqualsV( rotated2, vec3( 0, 0, 1 ), 0.0001f ) );
	}
}
TEST_CASE( "quaternion multiplication", "[quat]" )
{
	using namespace etm;

	quat R1 = quat( vec3( 0, 0, 1 ), etm::PI_DIV2 );
	quat R2 = quat( vec3( 0, 1, 0 ), etm::PI_DIV2 );

	SECTION( "quat mul quat" )
	{
		quat testQuat = R2 * R1;//right quat will be applied first

		vec3 initial = vec3( 0, 1, 0 );
		vec3 rotated = testQuat * initial;

		REQUIRE( nearEqualsV( rotated, vec3( 0, 0, 1 ), 0.0001f ) );
	}
	SECTION( "inverse" )
	{
		quat testQuat = R1 * inverse(R1) * R2;

		REQUIRE( nearEqualsV( R2.v4, testQuat.v4, 0.0001f ) );
	}
	SECTION( "conjugate" )
	{
		quat testQuat = R1 * inverse(R1) * R2;

		REQUIRE( nearEqualsV( R2.v4, testQuat.v4, 0.0001f ) );
	}
	SECTION( "inverse" )
	{
		REQUIRE( nearEqualsV( inverse(R1).v4, inverseSafe(R1).v4, 0.0001f ) );
	}
}
TEST_CASE( "matrix compatibility", "[quat]" )
{
	using namespace etm;

	quat R1 = quat( vec3( 0, 0, 1 ), etm::PI_DIV2 );
	mat3 r1m = R1.ToMatrix();

	SECTION( "to mat 3" )
	{
		mat3 transform( { 0, -1, 0,
						1, 0, 0,
						0, 0, 1 } );
		REQUIRE( nearEqualsM( r1m, transform, 0.00001f ) );
	}
	SECTION( "equal rotation" )
	{
		vec3 initV = vec3( 0, 1, 0 );
		vec3 qRot = R1 * initV;
		vec3 mRot = r1m * initV;
		REQUIRE( nearEqualsV( qRot, mRot, 0.0001f ) );
	}
}