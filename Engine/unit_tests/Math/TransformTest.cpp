#include <catch2/catch.hpp>

#include <EtMath/Transform.h>


using namespace et;


TEST_CASE("scale", "[transform]")
{
	SECTION("2D scale")
	{
		vec2 testVec = vec2( 1, 0 );
		mat2 scaleMat = math::scale( vec2( 3 ) );
		SECTION("no ref function")
		{
			vec2 resultVec = scaleMat * testVec;
			REQUIRE( math::nearEqualsV( resultVec, vec2( 3, 0 ), 0.0001f ) );
		}
		SECTION("ref function")
		{
			math::scale( scaleMat, vec2( 0.5 ) );
			vec2 resultVec = scaleMat * testVec;
			REQUIRE( math::nearEqualsV( resultVec, vec2( 1.5f, 0 ), 0.0001f ) );
		}
	}
	//3d vectors create 4x4 matricies
	SECTION("3D scale")
	{
		vec3 testVec = vec3( 1, 0, 0 );
		mat4 scaleMat = math::scale( vec3( 3 ) );
		SECTION("no ref function")
		{
			vec3 resultVec = (scaleMat * vec4(testVec, 0)).xyz;
			REQUIRE( math::nearEqualsV( resultVec, vec3( 3, 0, 0 ), 0.0001f ) );
		}
		SECTION("ref function")
		{
			math::scale( scaleMat, vec3( 0.5 ) );
			vec3 resultVec = (scaleMat * vec4(testVec, 0)).xyz;
			REQUIRE( math::nearEqualsV( resultVec, vec3( 1.5f, 0, 0 ), 0.0001f ) );
		}
	}
}

TEST_CASE("rotate", "[transform]")
{
	vec3 testVec = vec3( 1, 0, 0 );
	mat4 rotMat = math::rotate( vec3( 0, 0, 1 ), math::PI_DIV2 );
	SECTION("from axis angle")
	{
		vec3 resultVec = (rotMat * vec4(testVec, 0)).xyz;
		REQUIRE( math::nearEqualsV( resultVec, vec3( 0, 1, 0 ), 0.0001f ) );
	}
	SECTION("ref function")
	{
		math::rotate( rotMat, vec3( 0, 0, 1 ), -math::PI_DIV2 );
		vec3 resultVec = (rotMat * vec4(testVec, 0)).xyz;
		REQUIRE( math::nearEqualsV( resultVec, testVec, 0.0001f ) );
	}
	SECTION( "quaternions" )
	{
		quat rotQuat = quat( vec3( 0, 0, 1 ), math::PI_DIV2 );
		SECTION("quaterion rotation compatibility")
		{
			vec3 resultVec = (rotMat * vec4(testVec, 0)).xyz;
			vec3 quatResultVec = rotQuat * testVec;

			REQUIRE( math::nearEqualsV( resultVec, quatResultVec, 0.0001f ) );
		}
		SECTION("from quaterion")
		{
			mat4 quatRotMat = math::rotate( rotQuat );
			REQUIRE( math::nearEqualsM( quatRotMat, rotMat, 0.0001f ) );

			REQUIRE( math::nearEqualsM( rotQuat.ToMatrix(), math::CreateFromMat4( quatRotMat ), 0.0001f ) );

			vec3 resultVec = (quatRotMat * vec4(testVec, 0)).xyz;
			REQUIRE( math::nearEqualsV( resultVec, vec3( 0, 1, 0 ), 0.0001f ) );
		}
		SECTION("ref function")
		{
			mat4 quatRotMat = math::rotate( rotQuat );
			math::rotate( quatRotMat, math::inverse( rotQuat ) );
			vec3 resultVec = (quatRotMat * vec4(testVec, 0)).xyz;
			REQUIRE( math::nearEqualsV( resultVec, testVec, 0.0001f ) );
		}
	}
}

TEST_CASE( "translate", "[transform]" )
{
	vec3 testVec = vec3( 1, 0, 0 );
	mat4 moveMat = math::translate( vec3( 0, 0, 1 ) );
	SECTION( "no ref" )
	{
		vec3 resultVec = (moveMat * vec4( testVec, 1 )).xyz;
		REQUIRE( math::nearEqualsV( resultVec, vec3( 1, 0, 1 ), 0.0001f ) );
	}
	SECTION( "with ref" )
	{
		math::translate( moveMat, vec3( 0, 0, -1 ) );
		vec3 resultVec = (moveMat * vec4( testVec, 0 )).xyz;
		REQUIRE( math::nearEqualsV( resultVec, testVec, 0.0001f ) );
	}
}

TEST_CASE( "complete transform", "[transform]" )
{
	vec3 testVec = vec3( 0, 1, 0 );

	vec3 origScale = vec3(3);
	quat origRot = quat(vec3(0, 0, 1), -math::PI_DIV2);
	vec3 origTrans = vec3(0, 0, 1);

	mat4 scaleMat = math::scale(origScale);
	mat4 rotMat = math::rotate(origRot);
	mat4 moveMat = math::translate(origTrans);

	mat4 transformMat = scaleMat * rotMat * moveMat;

	SECTION("construct")
	{
		vec3 resultVec = (transformMat * vec4( testVec, 1 )).xyz;

		REQUIRE( math::nearEqualsV( resultVec, vec3(3, 0, 1), 0.00001f ) );
	}

	SECTION("decompose")
	{
		vec3 newScale;
		quat newRot;
		vec3 newTrans;

		math::decomposeTRS(transformMat, newTrans, newRot, newScale);

		REQUIRE(math::nearEqualsV(origTrans, newTrans, 0.00001f));
		REQUIRE(math::nearEqualsV(origScale, newScale, 0.00001f));

		REQUIRE(math::nearEqualsV(origRot * testVec, newRot * testVec, 0.00001f));
	}
}

TEST_CASE( "look at", "[transform]" )
{
	vec3 testVec = vec3::FORWARD;
	vec3 upTestVec = vec3::UP;

	vec3 eyePos = vec3::ZERO;

	vec3 targetPos = vec3( 5.3f, 2.f, 1.2f );
	vec3 targetVec = math::normalize(targetPos - eyePos);

	mat4 lookMat = math::lookAt( eyePos, targetPos, vec3::UP);

	vec3 resultVec = math::normalize((math::inverse(lookMat) * vec4( testVec, 1 )).xyz);
	vec3 resultUpVec = (math::inverse(lookMat) * vec4( upTestVec, 1 )).xyz;

	REQUIRE( math::nearEqualsV( resultVec, targetVec, 0.00001f ) );
	REQUIRE( math::dot(resultUpVec, vec3::UP) >= 0 );
	REQUIRE( math::nearEquals( math::determinant( math::CreateFromMat4( lookMat ) ), 1.f, 0.00001f ) );
}

TEST_CASE( "projection", "[transform]" )
{
	float width = 16.f;
	float height = 9.f;
	float zNear = 1.f;
	float zFar = 10.f;

	SECTION( "ortho" )
	{
		float hw = width / 2;
		float hh = height / 2;
		mat4 projMat = math::orthographic( -hw, hw, hh, -hh, zNear, zFar );

		vec3 bln = vec3( -hw, -hh, zNear );
		vec3 tln = vec3( -hw, hh, zNear );
		vec3 brn = vec3( hw, -hh, zNear );
		vec3 trn = vec3( hw, hh, zNear );
		vec3 blf = vec3( -hw, -hh, zFar );
		vec3 tlf = vec3( -hw, hh, zFar );
		vec3 brf = vec3( hw, -hh, zFar );
		vec3 trf = vec3( hw, hh, zFar );

		vec4 rbln = projMat * vec4( bln, 1 );
		vec4 rtln = projMat * vec4( tln, 1 );
		vec4 rbrn = projMat * vec4( brn, 1 );
		vec4 rtrn = projMat * vec4( trn, 1 );
		vec4 rblf = projMat * vec4( blf, 1 );
		vec4 rtlf = projMat * vec4( tlf, 1 );
		vec4 rbrf = projMat * vec4( brf, 1 );
		vec4 rtrf = projMat * vec4( trf, 1 );

		REQUIRE( math::nearEqualsV( rbln.xyz, vec3(-1, -1, -1), 0.0001f ) );
		REQUIRE( math::nearEqualsV( rtln.xyz, vec3(-1, 1, -1), 0.0001f ) );
		REQUIRE( math::nearEqualsV( rbrn.xyz, vec3(1, -1, -1), 0.0001f ) );
		REQUIRE( math::nearEqualsV( rtrn.xyz, vec3(1, 1, -1), 0.0001f ) );
		REQUIRE( math::nearEqualsV( rblf.xyz, vec3(-1, -1, 1), 0.0001f ) );
		REQUIRE( math::nearEqualsV( rtlf.xyz, vec3(-1, 1, 1), 0.0001f ) );
		REQUIRE( math::nearEqualsV( rbrf.xyz, vec3(1, -1, 1), 0.0001f ) );
		REQUIRE( math::nearEqualsV( rtrf.xyz, vec3(1, 1, 1), 0.0001f ) );
	}
	SECTION( "perspective" )
	{
		float aspect = width / height;
		float fov = math::PI_DIV4;

		mat4 projMat = math::perspective( fov, aspect, zNear, zFar );

		//Test with frustum corners
		float yFac = tanf( fov / 2);
		float xFac = yFac*aspect;

		vec3 nCenter = vec3::ZERO + vec3::FORWARD*zNear;
		vec3 fCenter = vec3::ZERO + vec3::FORWARD*zFar;
		vec3 nearHW = vec3::RIGHT*zNear*xFac;
		vec3 nearHH = vec3::UP*zNear*yFac;
		vec3 farHW = vec3::RIGHT*zFar*xFac;
		vec3 farHH = vec3::UP*zFar*yFac;

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

		REQUIRE( math::nearEqualsV( nbln, vec3( -1, -1, -1 ), 0.0001f ) );
		REQUIRE( math::nearEqualsV( ntln, vec3( -1, 1, -1 ), 0.0001f ) );
		REQUIRE( math::nearEqualsV( nbrn, vec3( 1, -1, -1 ), 0.0001f ) );
		REQUIRE( math::nearEqualsV( ntrn, vec3( 1, 1, -1 ), 0.0001f ) );
		REQUIRE( math::nearEqualsV( nblf, vec3( -1, -1, 1 ), 0.0001f ) );
		REQUIRE( math::nearEqualsV( ntlf, vec3( -1, 1, 1 ), 0.0001f ) );
		REQUIRE( math::nearEqualsV( nbrf, vec3( 1, -1, 1 ), 0.0001f ) );
		REQUIRE( math::nearEqualsV( ntrf, vec3( 1, 1, 1 ), 0.0001f ) );
	}
}