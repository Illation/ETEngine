#pragma once

//#include "../Helper/AtomicTypes.h"
#include "Vector.hpp"
#include "Matrix.hpp"

namespace etm
{
	//Generic quaternion
	//******************

	//consists of a vec3 v and a scalar w
	//should always be normalized to represent a rotation

	template <class T>
	struct quaternion
	{
		//members
		union
		{
			//Standard representation
			struct
			{
				union
				{
					struct
					{
						T x, y, z;
					};
					vector<3, T> v;
				};
				T w;
			};
			//Vec4 representation
			//makes vector like operations easy to implement
			vector<4, T> v4; 
		};

		//constructors
		quaternion() : x(0), y(0), z(0), w(1) //Identity quaternion
		{}
		quaternion(const vector<3, T>& axis, const T angle)//Axis angle initialization
		{
			const T halfAngle = angle * static_cast<T>(0.5);
			const T sinHalf = sin(halfAngle);
			x = axis.x * sinHalf;
			y = axis.y * sinHalf;
			z = axis.z * sinHalf;
			w = cos(halfAngle);
		}
		quaternion(const T &lx, const T &ly, const T &lz, const T &lw) //value initialization
		{
			x = lx;
			y = ly;
			z = lz;
			w = lw;
		}
		explicit quaternion(const vector<3, T>& axis)//Axis initialization, angle is 0
		{
			x = axis.x;
			y = axis.y;
			z = axis.z;
			w = 0;
		}

		//Conversions
		vector<4, T> ToAxisAngle() const;
		matrix<3, 3, T> ToMatrix() const;
	};

	//shorthands
	typedef quaternion<float>  quat;
	typedef quaternion<double> quatd; //ultra precise rotations ? maybe someone wants to simulate a clockwork

	//operations 
	//Grassman product
	template <typename T>
	inline quaternion<T> operator*(const quaternion<T>& lhs, const quaternion<T>& rhs)
	{
		const T w = rhs.w*lhs.w - etm::dot(rhs.v, lhs.v);
		vector<3, T> v = rhs.w*lhs.v + lhs.w*rhs.v + etm::cross(lhs.v, rhs.v);

		return quaternion<T>(v.x, v.y, v.z, w);
	}

	//Rotate vector with quaternion
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
			+ static_cast<T>(2) * (dot( q.v, vec )*q.v + q.w*cross( q.v, vec ));
	}

	template <typename T>
	inline quaternion<T>& normalize(quaternion<T>& q)
	{
		q.v4 = etm::normalize(q.v4);
		return q;
	}

	//unit quaternion inversion - cheap assuming q is normalized
	template <typename T>
	inline quaternion<T> inverse(const quaternion<T>& q)
	{
		return quaternion<T>(-q.x, -q.y, -q.z, q.w);
	}
	template <typename T>
	inline quaternion<T> inverseSafe(const quaternion<T>& q)
	{
		auto result = quaternion<T>(-q.x, -q.y, -q.z, q.w);
		T len2 = etm::lengthSquared(q.v4);
		result.v4 = result.v4 * len2;

		return result;
	}

	//conversions

	template <typename T>
	inline vector<4, T> quaternion<T>::ToAxisAngle() const
	{
		vector<4, T> result;

		const T angle = static_cast<T>(2) * acos(w);
		const T rcpLen = static_cast<T>(1) / sqrt(static_cast<T>(1) - w*w);

		result.xyz = vector<3, T>(x, y, z) * rcpLen;
		result.w = angle;

		return result;
	}
	template <typename T>
	inline matrix<3, 3, T> quaternion<T>::ToMatrix() const //#todo implement the reverse function
	{
		matrix<3, 3, T> mat;

		mat[0][0] = static_cast<T>(1)	  - static_cast<T>(2)*y*y - static_cast<T>(2)*z*z;
		mat[0][1] = static_cast<T>(2)*x*y - static_cast<T>(2)*w*z;
		mat[0][2] = static_cast<T>(2)*x*z + static_cast<T>(2)*w*y;

		mat[1][0] = static_cast<T>(2)*x*y + static_cast<T>(2)*w*z;
		mat[1][1] = static_cast<T>(1)	  - static_cast<T>(2)*x*x - static_cast<T>(2)*z*z;
		mat[1][2] = static_cast<T>(2)*y*z - static_cast<T>(2)*w*x;

		mat[2][0] = static_cast<T>(2)*x*z - static_cast<T>(2)*w*y;
		mat[2][1] = static_cast<T>(2)*y*z + static_cast<T>(2)*w*x;
		mat[2][2] = static_cast<T>(1)	  - static_cast<T>(2)*x*x - static_cast<T>(2)*y*y;

		return mat;
	}

}//namespace etm