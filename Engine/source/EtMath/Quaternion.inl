#pragma once

// Inline functions
//////////////////////


namespace et {
namespace math {
	

//====================
// Quaternion
//====================


template <class T>
quaternion<T>::quaternion() : x(0), y(0), z(0), w(1) //Identity quaternion
{ }

template <class T>
quaternion<T>::quaternion(const vector<3, T>& axis, const T angle) //Axis angle initialization
{
	const T halfAngle = angle * static_cast<T>(0.5);
	const T sinHalf = sin(halfAngle);
	x = axis.x * sinHalf;
	y = axis.y * sinHalf;
	z = axis.z * sinHalf;
	w = cos(halfAngle);
}

template <class T>
quaternion<T>::quaternion(const T &lx, const T &ly, const T &lz, const T &lw) //value initialization
{
	x = lx;
	y = ly;
	z = lz;
	w = lw;
}

template <class T>
quaternion<T>::quaternion(const vector<3, T>& euler) //Euler angle initialization
{
	vector<3, T> fac = euler * static_cast<T>(0.5);
	vector<3, T> c(cos(fac.x), cos(fac.y), cos(fac.z));
	vector<3, T> s(sin(fac.x), sin(fac.y), sin(fac.z));

	x = s.x * c.y * c.z - c.x * s.y * s.z;
	y = c.x * s.y * c.z + s.x * c.y * s.z;
	z = c.x * c.y * s.z - s.x * s.y * c.z;
	w = c.x * c.y * c.z + s.x * s.y * s.z;
}

template <class T>
quaternion<T>::quaternion(const matrix<3, 3, T>& rot) //Rotation matrix initialization
{
	uint8 biggestIndex = 0;
	T r012 = rot[0][0] - rot[1][1] - rot[2][2];
	T r102 = rot[1][1] - rot[0][0] - rot[2][2];
	T r201 = rot[2][2] - rot[0][0] - rot[1][1];
	T biggestVal = rot[0][0] + rot[1][1] + rot[2][2];
	if (r012 > biggestVal)
	{
		biggestVal = r012;
		biggestIndex = 1;
	}
	if (r102 > biggestVal)
	{
		biggestVal = r102;
		biggestIndex = 2;
	}
	if (r201 > biggestVal)
	{
		biggestVal = r201;
		biggestIndex = 3;
	}
	biggestVal = sqrt(biggestVal + static_cast<T>(1)) * static_cast<T>(0.5);
	T mult = static_cast<T>(0.25) / biggestVal;

	switch (biggestIndex)
	{
	case 0:
		x = (rot[2][1] - rot[1][2]) * mult;
		y = (rot[0][2] - rot[2][0]) * mult;
		z = (rot[1][0] - rot[0][1]) * mult;
		w = -biggestVal;
	case 1:
		x = biggestVal;
		y = (rot[1][0] - rot[0][1]) * mult;
		z = (rot[0][2] - rot[2][0]) * mult;
		w = -(rot[2][1] - rot[1][2]) * mult;
	case 2:
		x = (rot[1][0] - rot[0][1]) * mult;
		y = biggestVal;
		z = (rot[2][1] - rot[1][2]) * mult;
		w = -(rot[0][2] - rot[2][0]) * mult;
	case 3:
		x = (rot[0][2] - rot[2][0]) * mult;
		y = (rot[2][1] - rot[1][2]) * mult;
		z = biggestVal;
		w = -(rot[1][0] - rot[0][1]) * mult;
	}
}


template <typename T>
inline quaternion<T> operator*(const quaternion<T>& lhs, const quaternion<T>& rhs)
{
	const T w = rhs.w*lhs.w - math::dot(rhs.v, lhs.v);
	vector<3, T> v = rhs.w*lhs.v + lhs.w*rhs.v + math::cross(lhs.v, rhs.v);

	return quaternion<T>(v.x, v.y, v.z, w);
}

template <typename T>
inline vector<3, T> operator*(const quaternion<T>& q, const vector<3, T>& vec)
{
	// slow version but mathematically correct
	//quat vecQuat = quat( vec.x, vec.y, vec.z, 0 );
	//return (q * vecQuat * inverse( q )).v;

	//fast version
	//vector<3, T> temp = static_cast<T>(2) * cross( q.v, vec );
	//return vec + q.w * temp + cross( q.v, temp);

	//faster version
	//return static_cast<T>(2) * dot( q.v, vec ) * q.v
	//	+ (q.w * q.w - dot( q.v, q.v )) * vec
	//	+ static_cast<T>(2) * q.w * cross( q.v, vec );

	//fastest version
	return (static_cast<T>(2)*(q.w * q.w) - static_cast<T>(1))*vec
		+ static_cast<T>(2) * (dot(q.v, vec)*q.v + q.w*cross(q.v, vec));
}

template <typename T>
inline quaternion<T> & normalize(quaternion<T>& q)
{
	q.v4 = math::normalize(q.v4);
	return q;
}

template <typename T>
inline quaternion<T> inverse(const quaternion<T>& q)
{
	return quaternion<T>(-q.x, -q.y, -q.z, q.w);
}

template <typename T>
inline quaternion<T> inverseSafe(const quaternion<T>& q)
{
	auto result = quaternion<T>(-q.x, -q.y, -q.z, q.w);
	T len2 = math::lengthSquared(q.v4);
	result.v4 = result.v4 * len2;

	return result;
}

//conversions

template <typename T>
inline vector<4, T> quaternion<T>::ToAxisAngle() const
{
	vector<4, T> result;

	const T angle = static_cast<T>(2) * acos(w);
	const T rcpLen = static_cast<T>(1) / sqrt(static_cast<T>(1) - w * w);

	result.xyz = vector<3, T>(x, y, z) * rcpLen;
	result.w = angle;

	return result;
}
template <class T>
math::vector<3, T> math::quaternion<T>::ToEuler() const
{
	const T qxx(x * x);
	const T qyy(y * y);
	const T qzz(z * z);
	const T qww(w * w);

	T pitch = atan2(static_cast<T>(2) * (y*z + w * x), qww - qxx - qyy + qzz);
	T roll = atan2(static_cast<T>(2) * (x*y + w * z), qww + qxx - qyy - qzz);

	return vector<3, T>(pitch, Yaw(), roll);
}
template <class T>
T math::quaternion<T>::Pitch() const
{
	return atan2(static_cast<T>(2) * (y*z + w * x), w * w - x * x - y * y + z * z);
}
template <class T>
T math::quaternion<T>::Yaw() const
{
	return asin(math::Clamp(static_cast<T>(-2) * (x*z - w * y), static_cast<T>(-1), static_cast<T>(1)));
}
template <class T>
T math::quaternion<T>::Roll() const
{
	return atan2(static_cast<T>(2) * (x*y + w * z), w * w + x * x - y * y - z * z);
}
template <typename T>
inline matrix<3, 3, T> quaternion<T>::ToMatrix() const //#todo implement the reverse function
{
	matrix<3, 3, T> mat;

	mat[0][0] = static_cast<T>(1) - static_cast<T>(2)*y*y - static_cast<T>(2)*z*z;
	mat[1][0] = static_cast<T>(2)*x*y - static_cast<T>(2)*w*z;
	mat[2][0] = static_cast<T>(2)*x*z + static_cast<T>(2)*w*y;

	mat[0][1] = static_cast<T>(2)*x*y + static_cast<T>(2)*w*z;
	mat[1][1] = static_cast<T>(1) - static_cast<T>(2)*x*x - static_cast<T>(2)*z*z;
	mat[2][1] = static_cast<T>(2)*y*z - static_cast<T>(2)*w*x;

	mat[0][2] = static_cast<T>(2)*x*z - static_cast<T>(2)*w*y;
	mat[1][2] = static_cast<T>(2)*y*z + static_cast<T>(2)*w*x;
	mat[2][2] = static_cast<T>(1) - static_cast<T>(2)*x*x - static_cast<T>(2)*y*y;

	return mat;
}

template <class T>
std::string math::quaternion<T>::ToString() const
{
	return std::string("[") + std::to_string(x) + ", " + std::to_string(y)
		+ ", " + std::to_string(z) + " - " + std::to_string(w) + "]";
}

template <class T>
std::ostream& operator<<(std::ostream& os, quaternion<T>& vec)
{
	return os << vec.ToString();
}

} // namespace math
} // namespace et