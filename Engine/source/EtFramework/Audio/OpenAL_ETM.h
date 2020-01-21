#pragma once


namespace et {
namespace fw {


inline vec3 ALvec3(const vec3 &rhs)
{
	return vec3(rhs.x, rhs.y, -rhs.z);
}


} // namespace fw
} // namespace et