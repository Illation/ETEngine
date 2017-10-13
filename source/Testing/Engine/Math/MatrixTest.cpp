#include <catch.hpp>

#include "../../../Engine/Math/Matrix.hpp"

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
TEST_CASE( "Matrix operations", "[matrix]" )
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

	SECTION( "+ operator" )
	{
		mat2 a = mat2( {in1, in2,
						in3, in4 } );
		mat2 b = mat2( {in5, in6,
						in7, in8 } );
		REQUIRE( nearEqualsM( a + b, mat2( {11.1f, 8.5f,
											6.2f,  7.2f } ) ) );
	}
	SECTION( "- operator" )
	{
		mat2 a = mat2( {in1, in2,
						in3, in4 } );
		mat2 b = mat2( {in5, in6,
						in7, in8 } );
		
		REQUIRE( nearEqualsM( a - b, mat2( {-0.5f, -6.1f,
											-0.2f, -2.6f } ), 0.00001f ) );
	}
	SECTION( "scalar multiplication" )
	{
		mat2 a = mat2( {in1, in2,
						in3, in4 } );
		mat2 b = mat2( {10.6f,	2.4f,
						6.f,	4.6f } );
		REQUIRE( nearEqualsM( a * 2.f, b ) );
	}
	SECTION( "vector multiplication" )
	{
		mat3 transform( {	0, 1, 0,
						   -1, 0, 0,
							0, 0, 1 } );
		vec3 initial( 1, 0, 0 );
		REQUIRE( nearEqualsV( transform * initial, vec3( 0, -1, 0 ) ) );
	}
	SECTION( "transpose" )
	{
		mat3 a( {	in1, in2, in3,
					in4, in5, in6,
					in7, in8, in9 } );
		mat3 b( {	in1, in4, in7,
					in2, in5, in8,
					in3, in6, in9 } );
		REQUIRE( nearEqualsM( transpose( a ), b ) );
	}
}
TEST_CASE( "Specialized matrix functionality", "[matrix]" )
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

	SECTION( "mat4 -> mat3" )
	{
		vec4 v1( in1, in2, in3, 32.f );
		vec4 v2( in4, in5, in6, 23.f );
		vec4 v3( in7, in8, in9, 21.f );
		vec4 v4( in7, in2, in4, 54.f );
		mat4 initialMat( new vec4[4]{ v1, v2, v3, v4 } );
		mat3 mat = CreateFromMat4( initialMat );
		REQUIRE( nearEqualsV( mat[0], v1.xyz ) );
		REQUIRE( nearEqualsV( mat[1], v2.xyz ) );
		REQUIRE( nearEqualsV( mat[2], v3.xyz ) );
	}

	SECTION( "mat3 -> mat4" )
	{
		vec3 v1( in1, in2, in3 );
		vec3 v2( in4, in5, in6 );
		vec3 v3( in7, in8, in9 );
		mat3 initialMat( new vec3[3]{ v1, v2, v3 } );
		mat4 mat = CreateFromMat3( initialMat );
		REQUIRE( nearEqualsV( mat[0], vec4( v1, 0)));
		REQUIRE( nearEqualsV( mat[1], vec4( v2, 0)));
		REQUIRE( nearEqualsV( mat[2], vec4( v3, 0)));
		REQUIRE( nearEqualsV( mat[3], vec4( 0, 0, 0, 1)));
	}

	SECTION( "lose translation" )
	{
		vec4 v1( in1, in2, in3, 32.f );
		vec4 v2( in4, in5, in6, 23.f );
		vec4 v3( in7, in8, in9, 21.f );
		vec4 v4( in7, in2, in4, 54.f );
		mat4 initialMat( new vec4[4]{ v1, v2, v3, v4 } );
		mat4 mat = DiscardW( initialMat );
		REQUIRE( nearEqualsV( mat[0], vec4( v1.xyz, 0 ) ) );
		REQUIRE( nearEqualsV( mat[1], vec4( v2.xyz, 0 ) ) );
		REQUIRE( nearEqualsV( mat[2], vec4( v3.xyz, 0 ) ) );
		REQUIRE( nearEqualsV( mat[3], vec4( 0, 0, 0, 1 ) ) );

		REQUIRE( nearEqualsM( mat, CreateFromMat3(CreateFromMat4(initialMat))));
	}

	SECTION( "mat2 inverse, determinant and multiplication" )
	{
		vec2 v1( in1, 0.f );
		vec2 v2( in4, 1.f );
		mat2 initialMat( new vec2[2]{ v1, v2 } );
		mat2 inverseMat = inverse( initialMat );
		REQUIRE( nearEqualsM( mat2(), initialMat * inverseMat) ); //Matrix * inverse = identity
		REQUIRE( nearEqualsM( initialMat, inverse( inverseMat) ) ); //inverse(inverse) = initial
	}
	SECTION( "mat3 inverse, determinant and multiplication" )
	{
		vec3 v1( in1, in2, 0.f );
		vec3 v2( in4, in5, 0.f );
		vec3 v3( in7, in8, 1.f );
		mat3 initialMat( new vec3[3]{ v1, v2, v3 } );
		mat3 inverseMat = inverse( initialMat );
		REQUIRE( nearEqualsM( mat3(), initialMat * inverseMat, 0.00001f )); //Matrix * inverse = identity
		REQUIRE( nearEqualsM( initialMat, inverse( inverseMat ), 0.00001f )); //inverse(inverse) = initial
	}
	SECTION( "mat4 inverse and multiplication" )
	{
		vec4 v1( in1, in2, in3, 0.f );
		vec4 v2( in4, in5, in6, 0.f );
		vec4 v3( in7, in8, in9, 0.f );
		vec4 v4( in7, in2, in4, 1.f );
		mat4 initialMat( new vec4[4]{ v1, v2, v3, v4 } );
		mat4 inverseMat = inverse( initialMat );
		REQUIRE( nearEqualsM( mat4(), initialMat * inverseMat, 0.00001f ) ); //Matrix * inverse = identity
		REQUIRE( nearEqualsM( initialMat, inverse( inverseMat), 0.00001f ) ); //inverse(inverse) = initial
	}
	SECTION( "mat4 determinant" )
	{
		vec4 v1( in1, in2, in3, 0.f );
		vec4 v2( in4, in5, in6, 0.f );
		vec4 v3( in7, in8, in9, 0.f );
		vec4 v4( in7, in2, in4, 1.f );

		mat4 initialMat( new vec4[4]{ v1, v2, v3, v4 } );
		float det = determinant( initialMat );
		float manualDet =
			in1 * determinant( mat3( {	in5, in6, 0.f,
										in8, in9, 0.f,
										in2, in4, 1.f } ) ) -
			in2 * determinant( mat3( {	in4, in6, 0.f,
										in7, in9, 0.f,
										in7, in4, 1.f } ) ) +
			in3 * determinant( mat3( {	in4, in5, 0.f,
										in7, in8, 0.f,
										in7, in2, 1.f } ) ) -
			0.f * determinant( mat3( {	in4, in5, in6,
										in7, in8, in9,
										in7, in2, in4 } ) );

		REQUIRE( nearEquals( det, manualDet, 0.00001f ) );
	}
}