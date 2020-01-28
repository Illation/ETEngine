#pragma once
#pragma warning(disable : 4201) //nameless struct union - used in math library

#include "MathUtil.h"

#include <initializer_list>
#include <array>


namespace et {
namespace math {


enum ctor { uninitialized };

//Generic vector
//**************

// n = dimensions
// T = data type

// In 3D cartesian coordinate space:
// Left handed - Y is up

// Vectors are treated as columns when multiplied with matrices

template <uint8 n, class T>
struct vector
{
public:
	//members
	std::array<T, n> data;

	//constructors
	vector();
	vector(const T &rhs);
	vector(const std::initializer_list<T> args);

	//operators
	T operator[] (const uint8 index) const;
	T& operator[] (const uint8 index);
	vector<n, T> operator-() const;

	//string conversion
	std::string ToString() const;
	operator std::string() const { return ToString(); }
};

//specializations
//***************

//vec2
template <typename T>
struct vector<2, T>
{
	union
	{
		std::array<T, 2> data;
		struct
		{
			T x;
			T y;
		};
	};
	vector();
	vector(const T &rhs);
	vector(const std::initializer_list<T> args);
	vector(const T& x, const T& y);
	//operators
	T operator[] (const uint8 index) const;
	T& operator[] (const uint8 index);
	vector<2, T> operator-() const;

	//string conversion
	std::string ToString() const;
	operator std::string() const { return ToString(); }
};

//vec3
template<typename T>
struct vector<3, T>
{
	union
	{
		std::array<T, 3> data;
		struct
		{
			T x;
			T y;
			T z;
		};
		
		struct
		{
			vector<2, T> xy;
			T _vec3_ignored_z;
		};
		struct
		{
			T _vec3_ignored_x;
			vector<2, T> yz;
		};
	};

	static vector<3, T> ZERO;
	static vector<3, T> UP;
	static vector<3, T> DOWN;
	static vector<3, T> LEFT;
	static vector<3, T> RIGHT;
	static vector<3, T> FORWARD;
	static vector<3, T> BACK;

	vector();
	vector(const T &rhs);
	vector(const std::initializer_list<T> args);
	vector(const T& x, const T& y, const T& z);
	vector(const vector<2, T>& vec, const T& z);
	vector(const T& x, const vector<2, T>& vec);

	//operators
	T operator[] (const uint8 index) const;
	T& operator[] (const uint8 index);
	vector<3, T> operator-() const;

	//string conversion
	std::string ToString() const;
	operator std::string() const { return ToString(); }
};

//vec4
template<typename T>
struct vector<4, T>
{
	union
	{
		std::array<T, 4> data;
		struct
		{
			T x;
			T y;
			T z;
			T w;
		};
		struct
		{
			T r;
			T g;
			T b;
			T a;
		};
		struct
		{
			vector<2, T> xy;
			T _vec4_ignored1_z;
			T _vec4_ignored1_w;
		};
		struct
		{
			T _vec4_ignored2_x;
			vector<2, T> yz;
			T _vec4_ignored2_w;
		};
		struct
		{
			T _vec4_ignored3_x;
			T _vec4_ignored3_y;
			vector<2, T> zw;
		};
		struct
		{
			vector<3, T> xyz;
			T _vec4_ignored4_w;
		};
		struct
		{
			vector<3, T> rgb;
			T _vec4_ignored_a;
		};
	};

	//constructors
	vector();
	vector(const T &rhs);
	vector(const std::initializer_list<T> args);
	//with scalars
	vector(const T& x, const T& y, const T& z, const T& w);
	//with vec2
	vector(const vector<2, T>& xy, const vector<2, T>& zw);
	vector(const vector<2, T>& xy, const T& z, const T& w);
	vector(const T& x, const T& y, const vector<2, T>& zw);
	vector(const T& x, const vector<2, T>& yz, const T& w);
	//with vec3
	vector(const vector<3, T>& xyz, const T& w);
	vector(const T& x, const vector<3, T>& yzw);

	//operators
	T operator[] (const uint8 index) const;
	T& operator[] (const uint8 index);
	vector<4, T> operator-() const;

	//string conversion
	std::string ToString() const;
	operator std::string() const { return ToString(); }
};


//operators
//*********
template <uint8 n, class T>
std::ostream& operator<<( std::ostream& os, const math::vector<n, T>& vec);

template <class T>
std::ostream& operator<<( std::ostream& os, const math::vector<2, T>& vec);

template <class T>
std::ostream& operator<<( std::ostream& os, const math::vector<3, T>& vec);

template <class T>
std::ostream& operator<<( std::ostream& os, const math::vector<4, T>& vec);

//negate

// addition
template <uint8 n, class T>
vector<n, T> operator+(const vector<n, T> &lhs, const T scalar);

template <uint8 n, class T>
vector<n, T> operator+(const T scalar, const vector<n, T> &rhs);

template <uint8 n, class T>
vector<n, T> operator+(const vector<n, T> &lhs, const vector<n, T> &rhs);

// subtraction
template <uint8 n, class T>
vector<n, T> operator-(const vector<n, T> &lhs, const T scalar);

template <uint8 n, class T>
vector<n, T> operator-(const vector<n, T> &lhs, const vector<n, T> &rhs);

// multiplication
template <uint8 n, class T>
vector<n, T> operator*(const vector<n, T> lhs, const T &scalar);

template <uint8 n, class T>
vector<n, T> operator*(const T scalar, const vector<n, T> &lhs);

template <uint8 n, class T>
vector<n, T> operator*(const vector<n, T> &lhs, const vector<n, T> &rhs); // hadamard product

//division
template <uint8 n, class T>
vector<n, T> operator/(const vector<n, T> &lhs, const T scalar);

template <uint8 n, class T>
vector<n, T> operator/(const T scalar, const vector<n, T> &rhs);

template <uint8 n, class T>
vector<n, T> operator/(const vector<n, T> &lhs, const vector<n, T> &rhs); //hadamard product

//operations
//**********
template <uint8 n, class T>
bool nearEqualsV(const vector<n, T> &lhs, const vector<n, T> &rhs, const T epsilon = ETM_DEFAULT_EPSILON_T );

template <uint8 n, class T>
bool isZero(const vector<n, T> &lhs, const T epsilon = static_cast<T>(0));

template <uint8 n, class T>
bool operator==(const vector<n, T> &lhs, const vector<n, T> &rhs);

template <uint8 n, class T>
T dot(const vector<n, T> &lhs, const vector<n, T> &rhs);

//this is important so we do template specaializations
template <class T>
T dot(const vector<2, T> &lhs, const vector<2, T> &rhs);

template <class T>
T dot(const vector<3, T> &lhs, const vector<3, T> &rhs);

template <class T>
T dot(const vector<4, T> &lhs, const vector<4, T> &rhs);

template <uint8 n, class T>
T lengthSquared(const vector<n, T> &vec);
template <uint8 n, class T>
T length(const vector<n, T> &vec);

template <uint8 n, class T>
T distance(const vector<n, T> &lhs, const vector<n, T> &rhs);

template <uint8 n, class T>
T distanceSquared(const vector<n, T> &lhs, const vector<n, T> &rhs);

template <uint8 n, class T>
vector<n, T> normalize(const vector<n, T> &vec);

template <uint8 n, class T>
vector<n, T> pow(const vector<n, T> &vec, T exponent);

//Vectors need to be prenormalized
//if input vectors are zero it will generate NaN
template <uint8 n, class T>
T angleFastUnsigned(const vector<n, T>& lhs, const vector<n, T>& rhs);

template <uint8 n, class T>
T angleSafeUnsigned(const vector<n, T>& lhs, const vector<n, T>& rhs);

//access to array for the graphics api
template<uint8 n, typename T>
T const* valuePtr( vector<n, T> const& vec );

template<uint8 n, typename T>
T* valuePtr( vector<n, T>& vec );

template<typename T, uint8 n, typename T2>
vector<n, T> vecCast(const vector<n, T2> &vec);

//dimension specific operations
//*****************************
//vec2
template<class T>
vector<2, T> perpendicular(const vector<2, T>& vec);

//Vectors need to be prenormalized
template<class T>
T angleSigned(const vector<2, T>& lhs, const vector<2, T>& rhs);

//vec3
//direction will be left handed (lhs = thumb, rhs = index, result -> middle finger);
template<class T>
vector<3, T> cross(const vector<3, T>& lhs, const vector<3, T>& rhs);

//inputs vectors must be prenormalized, 
//and, for accurate measurments the outAxis should also be normalized after
//if input vectors are zero it will generate NaN
template<class T>
T angleFastAxis(const vector<3, T>& lhs, const vector<3, T>& rhs, vector<3, T> &outAxis);

template<class T>
T angleSafeAxis(const vector<3, T>& lhs, const vector<3, T>& rhs, vector<3, T> &outAxis);


} // namespace math


//shorthands
typedef math::vector<2, float>  vec2;
typedef math::vector<3, float>  vec3;
typedef math::vector<4, float>  vec4;
typedef math::vector<2, math::int32>  ivec2;
typedef math::vector<3, math::int32>  ivec3;
typedef math::vector<4, math::int32>  ivec4;
typedef math::vector<2, double> dvec2;
typedef math::vector<3, double> dvec3;
typedef math::vector<4, double> dvec4;


} // namespace et


#include "Vector.inl"