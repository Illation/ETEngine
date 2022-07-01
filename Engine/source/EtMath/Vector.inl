#pragma once

#include <cmath>
// Inline functions
//////////////////////


namespace et {
namespace math {
	

//====================
// Vector
//====================


template <uint8 n, class T>
vector<n, T>::vector()
{
	for (uint8 i = 0; i < n; ++i)
	{
		data[i] = 0;
	}
}

template <uint8 n, class T>
vector<n, T>::vector(const T &rhs)
{
	for (auto& el : data)
	{
		el = rhs;
	}
}

template <uint8 n, class T>
vector<n, T>::vector(const std::initializer_list<T> args)
{
	assert(args.size() <= n);
	int32 index = 0;
	for (auto begin = args.begin(); begin != args.end(); ++begin)
	{
		data.at(index++) = *begin;
	}
}

template <uint8 n, class T>
T vector<n, T>::operator[] (const uint8 index) const
{
	assert(index >= 0 && index < n);
	return data.at(index);
}

template <uint8 n, class T>
T& vector<n, T>::operator[] (const uint8 index)
{
	assert(index >= 0 && index < n);
	return data.at(index);
}

template <uint8 n, class T>
inline vector<n, T> vector<n, T>::operator-() const
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = -data[i];
	}

	return result;
}

template <uint8 n, class T>
std::string math::vector<n, T>::ToString() const
{
	std::string ret = "[";
	for (uint8 i = 0; i < data.size(); ++i)
	{
		ret += (i == data.size() - 1) ? (data[i]) : (data[i] + ", ");
	}

	return ret + "]";
}

template <uint8 n, class T>
std::ostream& operator<<(std::ostream& os, const vector<n, T>& vec)
{
	return os << vec.ToString();
}


//====================
// Vector 2
//====================


template <typename T>
vector<2, T>::vector()
{
	data = { 0, 0 };
}

template <typename T>
vector<2, T>::vector(const T &rhs)
{
	data = { rhs, rhs };
}

template <typename T>
vector<2, T>::vector(const std::initializer_list<T> args)
{
	assert(args.size() <= 2);
	int32 index = 0;
	for (auto begin = args.begin(); begin != args.end(); ++begin)
	{
		data.at(index++) = *begin;
	}
}

template <typename T>
vector<2, T>::vector(const T& x, const T& y)
{
	data = { x, y };
}

template <typename T>
T vector<2, T>::operator[] (const uint8 index) const
{
	assert(index >= 0 && index < 2);
	return data.at(index);
}

template <typename T>
T& vector<2, T>::operator[] (const uint8 index)
{
	assert(index >= 0 && index < 2);
	return data.at(index);
}

template <class T>
inline vector<2, T> vector<2, T>::operator-() const
{
	return{ -x, -y };
}

template <class T>
std::string math::vector<2, T>::ToString() const
{
	return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + "]";
}

template <class T>
std::ostream& operator<<(std::ostream& os, const vector<2, T>& vec)
{
	return os << vec.ToString();
}


//====================
// Vector 3
//====================


template<typename T>
vector<3, T>::vector()
{
	data = { 0, 0, 0 };
}

template<typename T>
vector<3, T>::vector(const T &rhs)
{
	data = { rhs, rhs, rhs };
}

template<typename T>
vector<3, T>::vector(const std::initializer_list<T> args)
{
	assert(args.size() <= 3);
	int32 index = 0;
	for (auto begin = args.begin(); begin != args.end(); ++begin)
	{
		data.at(index++) = *begin;
	}
}

template<typename T>
vector<3, T>::vector(const T& x, const T& y, const T& z)
{
	data = { x, y, z };
}

template<typename T>
vector<3, T>::vector(const vector<2, T>& vec, const T& z)
{
	data = { vec.x, vec.y, z };
}

template<typename T>
vector<3, T>::vector(const T& x, const vector<2, T>& vec)
{
	data = { x, vec.x, vec.y };
}

template<typename T>
T vector<3, T>::operator[] (const uint8 index) const
{
	assert(index >= 0 && index < 3);
	return data.at(index);
}

template<typename T>
T& vector<3, T>::operator[] (const uint8 index)
{
	assert(index >= 0 && index < 3);
	return data.at(index);
}

template <class T>
inline vector<3, T> vector<3, T>::operator-() const
{
	return{ -x, -y, -z };
}

template <class T>
std::string math::vector<3, T>::ToString() const
{
	return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";
}

template <class T>
std::ostream& operator<<(std::ostream& os, const vector<3, T>& vec)
{
	return os << vec.ToString();
}

template<typename T> vector<3, T> vector<3, T>::ZERO = vector<3, T>(0.0, 0.0, 0.0);
template<typename T> vector<3, T> vector<3, T>::UP = vector<3, T>(0.0, 1.0, 0.0);
template<typename T> vector<3, T> vector<3, T>::DOWN = vector<3, T>(0.0, -1.0, 0.0);
template<typename T> vector<3, T> vector<3, T>::LEFT = vector<3, T>(-1.0, 0.0, 0.0);
template<typename T> vector<3, T> vector<3, T>::RIGHT = vector<3, T>(1.0, 0.0, 0.0);
template<typename T> vector<3, T> vector<3, T>::FORWARD = vector<3, T>(0.0, 0.0, 1.0);
template<typename T> vector<3, T> vector<3, T>::BACK = vector<3, T>(0.0, 0.0, -1.0);


//====================
// Vector 4
//====================


template<typename T>
vector<4, T>::vector()
{
	data = { 0, 0, 0, 0 };
}

template<typename T>
vector<4, T>::vector(const T &rhs)
{
	data = { rhs, rhs, rhs, rhs };
}

template<typename T>
vector<4, T>::vector(const std::initializer_list<T> args)
{
	assert(args.size() <= 4);
	int index = 0;
	for (auto begin = args.begin(); begin != args.end(); ++begin)
	{
		data.at(index++) = *begin;
	}
}

template<typename T>
vector<4, T>::vector(const T& x, const T& y, const T& z, const T& w)
{
	data = { x, y, z, w };
}

template<typename T>
vector<4, T>::vector(const vector<2, T>& xy, const vector<2, T>& zw)
{
	data = { xy.x, xy.y, zw.x, zw.y };
}

template<typename T>
vector<4, T>::vector(const vector<2, T>& xy, const T& z, const T& w)
{
	data = { xy.x, xy.y, z, w };
}

template<typename T>
vector<4, T>::vector(const T& x, const T& y, const vector<2, T>& zw)
{
	data = { x, y, zw.x, zw.y };
}

template<typename T>
vector<4, T>::vector(const T& x, const vector<2, T>& yz, const T& w)
{
	data = { x, yz.x, yz.y, w };
}

template<typename T>
vector<4, T>::vector(const vector<3, T>& xyz, const T& w)
{
	data = { xyz.x, xyz.y, xyz.z, w };
}

template<typename T>
vector<4, T>::vector(const T& x, const vector<3, T>& yzw)
{
	data = { x, yzw.x, yzw.y, yzw.z };
}

template<typename T>
T vector<4, T>::operator[] (const uint8 index) const
{
	assert(index >= 0 && index < 4);
	return data.at(index);
}

template<typename T>
T& vector<4, T>::operator[] (const uint8 index)
{
	assert(index >= 0 && index < 4);
	return data.at(index);
}

template <class T>
inline vector<4, T> vector<4, T>::operator-() const
{
	return{ -x, -y, -z, -w };
}

template <class T>
std::string math::vector<4, T>::ToString() const
{
	return std::string("[") + std::to_string(x) + ", " + std::to_string(y)
		+ ", " + std::to_string(z) + ", " + std::to_string(w) + "]";
}

template <class T>
std::ostream& operator<<(std::ostream& os, const vector<4, T>& vec)
{
	return os << vec.ToString();
}


//====================
// Operators
//====================


template <uint8 n, class T>
inline vector<n, T> operator+(const vector<n, T> &lhs, const T scalar)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = lhs[i] + scalar;
	}
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator+(const T scalar, const vector<n, T> &rhs)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
		result[i] = rhs[i] + scalar;
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator+(const vector<n, T> &lhs, const vector<n, T> &rhs)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
		result[i] = lhs[i] + rhs[i];
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator-(const vector<n, T> &lhs, const T scalar)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = lhs[i] - scalar;
	}
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator-(const vector<n, T> &lhs, const vector<n, T> &rhs)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = lhs[i] - rhs[i];
	}
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator*(const vector<n, T> lhs, const T &scalar)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = lhs[i] * scalar;
	}
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator*(const T scalar, const vector<n, T> &lhs)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = lhs[i] * scalar;
	}
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator*(const vector<n, T> &lhs, const vector<n, T> &rhs) // hadamard product
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = lhs[i] * rhs[i];
	}
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator/(const vector<n, T> &lhs, const T scalar)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = lhs[i] / scalar;
	}
	return result;
}

template <uint8 n, class T>
inline vector<n, T> operator/(const T scalar, const vector<n, T> &rhs)
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = scalar / rhs[i];
	}
	return result;
}

template <uint8 n, class T>
inline vector<n, T>
operator/(const vector<n, T> &lhs, const vector<n, T> &rhs) //hadamard product
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[i] = lhs[i] / rhs[i];
	}
	return result;
}

template <uint8 n, class T>
bool operator==(const vector<n, T> &lhs, const vector<n, T> &rhs)
{
	return nearEqualsV(lhs, rhs);
}


//====================
// operations
//====================


template <uint8 n, class T>
bool nearEqualsV(const vector<n, T> &lhs, const vector<n, T> &rhs, const T epsilon /*= ETM_DEFAULT_EPSILON_T */)
{
	for (uint8 i = 0; i < n; ++i)
	{
		if (!nearEquals(lhs[i], rhs[i], epsilon)) return false;
	}
	return true;
}

template <uint8 n, class T>
bool isZero(const vector<n, T> &lhs, const T epsilon /*= static_cast<T>(0)*/)
{
	return nearEqualsV(lhs, vector<n, T>(0), epsilon);
}

template <uint8 n, class T>
T dot(const vector<n, T> &lhs, const vector<n, T> &rhs)
{
	T result = {};
	for (uint8 i = 0; i < n; ++i)
	{
		result += lhs[i] * rhs[i];
	}
	return result;
}

template <class T>
T dot(const vector<2, T> &lhs, const vector<2, T> &rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <class T>
T dot(const vector<3, T> &lhs, const vector<3, T> &rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template <class T>
T dot(const vector<4, T> &lhs, const vector<4, T> &rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}


template <uint8 n, class T>
T lengthSquared(const vector<n, T> &vec)
{
	return math::dot(vec, vec);
}

template <uint8 n, class T>
T length(const vector<n, T> &vec)
{
	return sqrt(math::lengthSquared(vec));
}


template <uint8 n, class T>
T distance(const vector<n, T> &lhs, const vector<n, T> &rhs)
{
	return length(lhs - rhs);
}

template <uint8 n, class T>
T distanceSquared(const vector<n, T> &lhs, const vector<n, T> &rhs)
{
	return lengthSquared(lhs - rhs);
}

template <uint8 n, class T>
inline vector<n, T>
normalize(const vector<n, T> &vec)
{
	//assert(!nearEqualsV(vec, vector<n, T>(0), static_cast<T>(0)));
	T len = length(vec);
	return vec / len;
}

template <uint8 n, class T>
inline vector<n, T>
pow(const vector<n, T> &vec, T exponent)
{
	vector<n, T> result = vector<n, T>();
	for (uint8 i = 0; i < n; ++i)
		result[i] = std::pow(vec[i], exponent);
	return result;
}


template <uint8 n, class T>
T angleFastUnsigned(const vector<n, T>& lhs, const vector<n, T>& rhs)
{
	return acos(dot(lhs, rhs));
}

template <uint8 n, class T>
T angleSafeUnsigned(const vector<n, T>& lhs, const vector<n, T>& rhs)
{
	return angleFastUnsigned(normalize(lhs), normalize(rhs));
}


template <uint8 n, class T>
T vecProjectionFactor(vector<n, T> const& source, vector<n, T> const& target)
{
	return dot(source, target) / lengthSquared(target);
}

template <uint8 n, class T>
vector<n, T> vecProjection(vector<n, T> const& source, vector<n, T> const& target)
{
	return target * vecProjectionFactor(source, target);
}


template<uint8 n, typename T>
T const* valuePtr(vector<n, T> const& vec)
{
	return &(vec.data[0]);
}

template<uint8 n, typename T>
T* valuePtr(vector<n, T>& vec)
{
	return &(vec.data[0]);
}

template<typename T, uint8 n, typename T2>
inline vector<n, T> vecCast(const vector<n, T2> &vec)
{
	vector<n, T> ret;
	for (uint8 i = 0; i < n; ++i)
	{
		ret[i] = static_cast<T>(vec[i]);
	}

	return ret;
}

template <uint8 n, class T>
inline vector<n, T> swizzle(vector<n, T> const& vec, vector<n, int32> const& indices) 
{
	vector<n, T> result;
	for (uint8 i = 0; i < n; ++i)
	{
		result[indices[i]] = vec[i];
	}

	return result;
}


//==================================
// dimension specific operations
//==================================


template<class T>
inline vector<2, T> perpendicular(const vector<2, T>& vec)
{
	vector<2, T> result;
	result.x = -vec.y;
	result.y = vec.x;
	return result;
}

template<class T>
T angleSigned(const vector<2, T>& lhs, const vector<2, T>& rhs)
{
	return atan2(rhs.y, rhs.x) - atan2(lhs.y, lhs.x);
}

template<class T>
inline vector<3, T>
cross(const vector<3, T>& lhs, const vector<3, T>& rhs)
{
	vector<3, T> result;

	result.x = lhs.y*rhs.z - lhs.z*rhs.y;
	result.y = lhs.z*rhs.x - lhs.x*rhs.z;
	result.z = lhs.x*rhs.y - lhs.y*rhs.x;

	return result;
}

template<class T>
T angleFastAxis(const vector<3, T>& lhs, const vector<3, T>& rhs, vector<3, T> &outAxis)
{
	outAxis = cross(lhs, rhs);
	return angleFastUnsigned(lhs, rhs);
}

template<class T>
T angleSafeAxis(const vector<3, T>& lhs, const vector<3, T>& rhs, vector<3, T> &outAxis)
{
	vector<3, T> lhsN = normalize(lhs);
	vector<3, T> rhsN = normalize(rhs);
	T result = angleFastAxis(lhsN, rhsN, outAxis);
	outAxis = normalize(outAxis);
	return result;
}


} // namespace math
} // namespace et