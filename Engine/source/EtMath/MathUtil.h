#pragma once

#include <cstdint>
#include <cassert>
#include <string>

namespace etm
{
	//Integers
	typedef std::int8_t		int8;
	typedef std::int16_t	int16;
	typedef std::int32_t	int32;
	typedef std::int64_t	int64;

	typedef std::uint8_t	uint8;
	typedef std::uint16_t	uint16;
	typedef std::uint32_t	uint32;
	typedef std::uint64_t	uint64;

	//float
	static_assert (sizeof(float) == 4, "compiler uses unexpected size for single precision floats, 32 bit assumed");
	typedef float float32;

	static_assert (sizeof(double) == 8, "compiler uses unexpected size for double precision floats, 64 bit assumed");
	typedef double float64;

#define ETM_DEFAULT_EPSILON 0.00000001
#define ETM_DEFAULT_EPSILON_T static_cast<T>(ETM_DEFAULT_EPSILON)

	//180 degrees
	static const float PI = 3.1415926535897932384626433832795028841971693993751058209749445f;
	//360 degrees
	static const float PI2 = PI * 2;
	//90 degrees
	static const float PI_DIV2 = PI * 0.5f;
	//45 degrees
	static const float PI_DIV4 = PI * 0.25f;
	//30 degrees
	static const float PI_DIV6 = PI / 6.f;
	//1 degree
	static const float PI_DIV180 = PI / 180.f;

	template <class T>
	inline bool nearEquals(T lhs, T rhs, T epsilon = ETM_DEFAULT_EPSILON_T )
	{
		return abs(lhs - rhs) <= epsilon;
	}

	template<class T>
	inline T Clamp(const T &value, T const hi, T const lo)
	{
		assert( hi >= lo );
		T result = value;

		if (value > hi)
			result = hi;

		if (value < lo)
			result = lo;
		 
		return result;
	}
	template<class T>
	inline T Clamp01(T const& value)
	{
		return Clamp<T>(value, static_cast<T>(1), static_cast<T>(0));
	}

	template<class T>
	constexpr inline T radians( const T &rhs )
	{
		return rhs * static_cast<T>(etm::PI_DIV180);
	}

} // namespace etm