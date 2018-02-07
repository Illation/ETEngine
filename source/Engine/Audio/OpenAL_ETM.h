#pragma once
#include "../Math/Vector.hpp"

inline vec3 ALvec3(const vec3 &rhs)
{
	return vec3(rhs.x, rhs.y, -rhs.z);
}