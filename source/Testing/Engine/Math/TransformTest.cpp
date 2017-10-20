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

TEST_CASE( "look at", "[transform]" )
{
	using namespace etm;

	vec3 testVec = vec3::FORWARD;
	vec3 upTestVec = vec3::UP;

	vec3 eyePos = vec3::ZERO;

	vec3 targetPos = vec3( 5.3f, 2.f, 1.2f );
	vec3 targetVec = etm::normalize(targetPos - eyePos);

	mat4 lookMat = etm::lookAt( eyePos, targetPos, vec3::UP);

	vec3 resultVec = etm::normalize((lookMat * vec4( testVec, 1 )).xyz);
	vec3 resultUpVec = (lookMat * vec4( upTestVec, 1 )).xyz;

	REQUIRE( nearEqualsV( resultVec, targetVec, 0.00001f ) );
	REQUIRE( etm::dot(resultUpVec, vec3::UP) >= 0 );
	REQUIRE( nearEquals( etm::determinant( etm::CreateFromMat4( lookMat ) ), 1.f, 0.00001f ) );
}

TEST_CASE( "projection", "[transform]" )
{
	using namespace etm;

	float width = 16.f;
	float height = 9.f;
	float near = 1.f;
	float far = 10.f;

	SECTION( "ortho" )
	{
		float hw = width / 2;
		float hh = height / 2;
		mat4 projMat = etm::orthographic( -hw, hw, hh, -hh, near, far );

		vec3 bln = vec3( -hw, -hh, near );
		vec3 tln = vec3( -hw, hh, near );
		vec3 brn = vec3( hw, -hh, near );
		vec3 trn = vec3( hw, hh, near );
		vec3 blf = vec3( -hw, -hh, far );
		vec3 tlf = vec3( -hw, hh, far );
		vec3 brf = vec3( hw, -hh, far );
		vec3 trf = vec3( hw, hh, far );

		vec4 rbln = projMat * vec4( bln, 1 );
		vec4 rtln = projMat * vec4( tln, 1 );
		vec4 rbrn = projMat * vec4( brn, 1 );
		vec4 rtrn = projMat * vec4( trn, 1 );
		vec4 rblf = projMat * vec4( blf, 1 );
		vec4 rtlf = projMat * vec4( tlf, 1 );
		vec4 rbrf = projMat * vec4( brf, 1 );
		vec4 rtrf = projMat * vec4( trf, 1 );

		REQUIRE( nearEqualsV( rbln.xyz, vec3(-1, -1, -1), 0.0001f ) );
		REQUIRE( nearEqualsV( rtln.xyz, vec3(-1, 1, -1), 0.0001f ) );
		REQUIRE( nearEqualsV( rbrn.xyz, vec3(1, -1, -1), 0.0001f ) );
		REQUIRE( nearEqualsV( rtrn.xyz, vec3(1, 1, -1), 0.0001f ) );
		REQUIRE( nearEqualsV( rblf.xyz, vec3(-1, -1, 1), 0.0001f ) );
		REQUIRE( nearEqualsV( rtlf.xyz, vec3(-1, 1, 1), 0.0001f ) );
		REQUIRE( nearEqualsV( rbrf.xyz, vec3(1, -1, 1), 0.0001f ) );
		REQUIRE( nearEqualsV( rtrf.xyz, vec3(1, 1, 1), 0.0001f ) );
	}
	SECTION( "perspective" )
	{
		float aspect = width / height;
		float fov = etm::PI_DIV4;

		mat4 projMat = etm::perspective( fov, aspect, near, far );

		//Test with frustum corners
		float yFac = tanf( fov / 2);
		float xFac = yFac*aspect;

		vec3 nCenter = vec3::ZERO + vec3::FORWARD*near;
		vec3 fCenter = vec3::ZERO + vec3::FORWARD*far;
		vec3 nearHW = vec3::RIGHT*near*xFac;
		vec3 nearHH = vec3::UP*near*yFac;
		vec3 farHW = vec3::RIGHT*far*xFac;
		vec3 farHH = vec3::UP*far*yFac;

		vec3 tln = nCenter + nearHH - nearHW;
		vec3 trn = nCenter + nearHH + nearHW;
		vec3 bln = nCenter - nearHH - nearHW;
		vec3 brn = nCenter - nearHH + nearHW;
		vec3 tlf = fCenter + farHH - farHW;
		vec3 trf = fCenter + farHH + farHW;
		vec3 blf = fCenter - farHH - farHW;
		vec3 brf = fCenter - farHH + farHW;

		//simulate projection
		vec4 rbln = projMat * vec4( bln, 1 );
		vec4 rtln = projMat * vec4( tln, 1 );
		vec4 rbrn = projMat * vec4( brn, 1 );
		vec4 rtrn = projMat * vec4( trn, 1 );
		vec4 rblf = projMat * vec4( blf, 1 );
		vec4 rtlf = projMat * vec4( tlf, 1 );
		vec4 rbrf = projMat * vec4( brf, 1 );
		vec4 rtrf = projMat * vec4( trf, 1 );

		vec3 nbln = rbln.xyz / rbln.w;
		vec3 ntln = rtln.xyz / rtln.w; 
		vec3 nbrn = rbrn.xyz / rbrn.w; 
		vec3 ntrn = rtrn.xyz / rtrn.w; 
		vec3 nblf = rblf.xyz / rblf.w; 
		vec3 ntlf = rtlf.xyz / rtlf.w; 
		vec3 nbrf = rbrf.xyz / rbrf.w; 
		vec3 ntrf = rtrf.xyz / rtrf.w; 

		REQUIRE( nearEqualsV( nbln, vec3( -1, -1, -1 ), 0.0001f ) );
		REQUIRE( nearEqualsV( ntln, vec3( -1, 1, -1 ), 0.0001f ) );
		REQUIRE( nearEqualsV( nbrn, vec3( 1, -1, -1 ), 0.0001f ) );
		REQUIRE( nearEqualsV( ntrn, vec3( 1, 1, -1 ), 0.0001f ) );
		REQUIRE( nearEqualsV( nblf, vec3( -1, -1, 1 ), 0.0001f ) );
		REQUIRE( nearEqualsV( ntlf, vec3( -1, 1, 1 ), 0.0001f ) );
		REQUIRE( nearEqualsV( nbrf, vec3( 1, -1, 1 ), 0.0001f ) );
		REQUIRE( nearEqualsV( ntrf, vec3( 1, 1, 1 ), 0.0001f ) );
	}
}