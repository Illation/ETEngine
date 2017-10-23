#pragma once
//inspired by JoeyDeVries/Cell , g-truc/glm , Game Engine Architecture
//implementing a custom math library for the learning experience, completeness, control and easy build setup
//#todo SIMD support
//#todo move implementation to .inl files

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Transform.hpp"

namespace etm
{
	template<class T>
	constexpr inline T radians( const T &rhs )
	{
		return rhs * static_cast<T>(etm::PI_DIV180);
	}
} // namespace etm