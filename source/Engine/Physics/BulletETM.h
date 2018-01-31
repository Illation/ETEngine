#pragma once

#include <btBulletDynamicsCommon.h>

inline btVector3 ToBtVec3(const vec3 &rhs)
{
	return btVector3(rhs.x, rhs.y, rhs.z);
}
inline btVector4 ToBtVec4(const vec4 &rhs)
{
	return btVector4(rhs.x, rhs.y, rhs.z, rhs.w);
}
inline btQuaternion ToBtQuat(const quat &rhs)
{
	return btQuaternion(rhs.x, rhs.y, rhs.z, rhs.w);
}

inline vec3 ToEtmVec3(const btVector3 &rhs)
{
	return vec3(rhs.getX(), rhs.getY(), rhs.getZ());
}
inline vec4 ToEtmVec4(const btVector4 &rhs)
{
	return vec4(rhs.getX(), rhs.getY(), rhs.getZ(), rhs.getW());
}
inline quat ToEtmQuat(const btQuaternion &rhs)
{
	return quat(ToEtmVec3(rhs.getAxis()), rhs.getAngle());
}