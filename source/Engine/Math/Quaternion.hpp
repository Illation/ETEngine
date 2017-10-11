#pragma once

//#include "../Helper/AtomicTypes.h"
#include "Vector.h"
#include "Matrix.h"

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
			const T halfAngle = angle / 2; //Compiler should optimize this
			const T sinHalf = sin(halfAngle);
			x = axis.x * sinHalf;
			y = axis.y * sinHalf;
			z = axis.z * sinHalf;
			w = cos(halfAngle);
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
		const float w2 = q.w * q.w;

		return (w2*w2 - 1.0f)*vec + 2.0f*etm::dot(q.v, vec)*q.v + w2*etm::cross(q.v, vec);
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

		return result / len2;
	}

	//conversions

	template <typename T>
	inline vector<4, T> quaternion<T>::ToAxisAngle() const
	{
		vector<4, T> result;

		const T angle = 2.0f * acos(w);
		const T len = sqrt(1.0f - angle*angle);

		result.xyz = vector<3, T>(x, y, z) / len;
		result.w = angle;

		return result;
	}
	template <typename T>
	inline matrix<3, 3, T> quaternion<T>::ToMatrix() const //#todo implement the reverse function
	{
		matrix<3, 3, T> mat;

		mat[0][0] = 1 - 2*y*y - 2*z*z;
		mat[0][1] = 2*x*y + 2*w*z;
		mat[0][2] = 2*x*z - 2*w*y;

		mat[1][0] = 2*x*y - 2*w*z;
		mat[1][1] = 1 - 2*x*x - 2*z*z;
		mat[1][2] = 2*y*z + 2*w*x;

		mat[2][0] = 2*x*z + 2*w*y;
		mat[2][1] = 2*y*z - 2*w*x;
		mat[2][2] = 1 - 2*x*x - 2*y*y;

		return mat;
	}

}//namespace etm