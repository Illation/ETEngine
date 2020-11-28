#pragma once
#pragma warning(disable : 4201) //nameless struct union - used in math library

#include "Vector.h"
#include "Matrix.h"

namespace et {
namespace math {


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
		std::array<T, 4> data;
	};

	//constructors
	quaternion(); //Identity quaternion
	quaternion(const vector<3, T>& axis, const T angle);//Axis angle initialization

	quaternion(const T &lx, const T &ly, const T &lz, const T &lw); //value initialization

	explicit quaternion(const vector<3, T>& euler);//Euler angle initialization

	quaternion(const matrix<3, 3, T>& rot);//Rotation matrix initialization

	//Conversions
	vector<4, T> ToAxisAngle() const;
	vector<3, T> ToEuler() const;
	T Pitch() const;
	T Yaw() const;
	T Roll() const;
	matrix<3, 3, T> ToMatrix() const;
	std::string ToString() const;
};

//operations 
//Grassman product
template <typename T>
quaternion<T> operator*(const quaternion<T>& lhs, const quaternion<T>& rhs);

//Rotate vector with quaternion
template <typename T>
vector<3, T> operator*(const quaternion<T>& q, const vector<3, T>& vec);

template <typename T>
quaternion<T>& normalize(quaternion<T>& q);

//unit quaternion inversion - cheap assuming q is normalized
template <typename T>
quaternion<T> inverse(const quaternion<T>& q);

template <typename T>
quaternion<T> inverseSafe(const quaternion<T>& q);

template <class T>
std::ostream& operator<<( std::ostream& os, math::quaternion<T>& vec);


} // namespace math


// shorthands
typedef math::quaternion<float>  quat;
typedef math::quaternion<double> quatd; //ultra precise rotations ? maybe someone wants to simulate a clockwork


} // namespace et

#include "Quaternion.inl"