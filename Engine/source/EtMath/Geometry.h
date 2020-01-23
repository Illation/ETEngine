#pragma once

#include "Vector.h"
#include "MathUtil.h"

#include <vector>


namespace et {
namespace math {


//------------------------
// Plane
//
// Mathematical definition for a plane
//
struct Plane
{
	Plane()
	{
		n = vec3(0, 1, 0);
		d = vec3(0, 0, 0);
	}
	Plane(vec3 normal, vec3 det)
	{
		n = normal;
		d = det;
	}
	Plane(vec3 a, vec3 b, vec3 c)
	{
		d = a;
		n = math::normalize(math::cross(-b + a, c - a));
	}
	vec3 n;
	vec3 d;
};

//------------------------
// Sphere
//
// Mathematical definition for a sphere
//
struct Sphere
{
	Sphere()
	{
		radius = 1;
		pos = vec3(0, 0, 0);
	}
	Sphere(vec3 position, float size)
	{
		pos = position;
		radius = size;
	}
	vec3 pos;
	float radius;
};


std::vector<vec3> GetIcosahedronPositions(float size = 1);
std::vector<math::uint32> GetIcosahedronIndices();//For inverse winding
std::vector<math::uint32> GetIcosahedronIndicesBFC();//for uniform winding


} // namespace math
} // namespace et
