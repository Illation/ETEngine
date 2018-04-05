#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"


//ETEngine math
namespace etm
{

	//scaling
	//*******
	template <uint8 n, class T>
	void scale(matrix<n, n, T>& result, const vector<n, T> &scaleVec )
	{
		for (uint8 i = 0; i < n; ++i)
		{
			result[i][i] *= scaleVec[i];
		}
	}
	template <uint8 n, class T>
	inline matrix<n, n, T> scale(const vector<n, T>& scaleVec)
	{
		matrix<n, n, T> mat;
		scale(mat, scaleVec );
		return mat;
	}
	//specialization for 3 dimensions
	template <class T>
	inline matrix<4, 4, T> scale(const vector<3, T> &scaleVec )
	{
		return scale(vector<4, T>( scaleVec, 1));
	}
	template <class T>
	void scale(matrix<4, 4, T>& result, const vector<3, T>& scaleVec )
	{
		scale(result, vector<4, T>( scaleVec, 1));
	}

	//rotation
	//********
	// #todo support euler rotations - for now can be done from a quaternion
	// #todo support 2D rotations
	template <class T>
	matrix<4, 4, T> rotate(const quaternion<T>& rotation)
	{
		matrix<3, 3, T> result = rotation.ToMatrix();
		return CreateFromMat3(result);
	}
	template <class T>
	void rotate(matrix<4, 4, T> &result, const quaternion<T>& rotation)
	{
		matrix<4, 4, T> rot = rotate(rotation);
		result = result * rot;
	}
	template <class T>
	matrix<4, 4, T> rotate(const vector<3, T>& axis, const T& angle)
	{
		matrix<4, 4, T> result;

		const float c = cos(angle);
		const float s = sin(angle);
		const float t = static_cast<T>(1) - c;

		result[0][0] = t*axis.x*axis.x + c;
		result[1][0] = t*axis.x*axis.y - s*axis.z;
		result[2][0] = t*axis.x*axis.z + s*axis.y;

		result[0][1] = t*axis.x*axis.y + s*axis.z;
		result[1][1] = t*axis.y*axis.y + c;
		result[2][1] = t*axis.y*axis.z - s*axis.x;

		result[0][2] = t*axis.x*axis.z - s*axis.y;
		result[1][2] = t*axis.y*axis.z + s*axis.x;
		result[2][2] = t*axis.z*axis.z + c;

		return result;
	}
	template <class T>
	void rotate(matrix<4, 4, T> &result, const vector<3, T>& axis, const T& angle)
	{
		matrix<4, 4, T> rot = rotate(axis, angle);
		result = result * rot;
	}

	//translation
	//***********
	template <class T>
	matrix<4, 4, T> translate(const vector<3, T>& translation)
	{
		matrix<4, 4, T> mat;
		mat[3] = vector<4, T>( translation, static_cast<T>(1) );
		return mat;
	}
	template <class T>
	void translate(matrix<4, 4, T> &result, const vector<3, T>& translation)
	{
		result =  result * translate(translation);
	}

	//look at
	//*******
	template <class T>
	matrix<4, 4, T> lookAt(const vector<3, T>& position, const vector<3, T>& target, const vector<3, T>& worldUp)
	{
		vector<3, T> forward = normalize(target - position);
		vector<3, T> right = normalize(cross(worldUp, forward));
		vector<3, T> up = cross(forward, right);

		matrix<4, 4, T> frame(uninitialized);
		frame[0] = vector<4, T>(right.x, up.x, forward.x, 0);
		frame[1] = vector<4, T>(right.y, up.y, forward.y, 0);
		frame[2] = vector<4, T>(right.z, up.z, forward.z, 0);
		frame[3] = vector<4, T>( -dot( right, position ), -dot( up, position ), -dot( forward, position ), 1 );

		return frame;
	}

	//projection
	//**********
	template <class T>
	matrix<4, 4, T> orthographic(const T& left, const T& right, const T& top, const T& bottom, const T& nearZ, const T& farZ )
	{
		matrix<4, 4, T> result;

		result[0][0] = static_cast<T>(2) / (right - left);
		result[1][1] = static_cast<T>(2) / (top - bottom);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);

		result[2][2] = static_cast<T>(2) / (farZ - nearZ);
		result[3][2] = -(farZ + nearZ) / (farZ - nearZ);
		
		return result;
	}
	template <class T>
	matrix<4, 4, T> perspective(const T& fov, const T& aspect, const T& nearZ, const T& farZ)
	{
		assert( std::abs( aspect - ETM_DEFAULT_EPSILON ) > static_cast<T>(0) );
		matrix<4, 4, T> result;

		T const tanHalfFov = tan( fov / static_cast<T>(2));

		result[0][0] = static_cast<T>(1) / (aspect * tanHalfFov);
		result[1][1] = static_cast<T>(1) / (tanHalfFov);
		result[2][3] = static_cast<T>(1);

		result[2][2] = (farZ + nearZ) / (farZ - nearZ);
		result[3][2] = (static_cast<T>(-2) * farZ * nearZ) / (farZ - nearZ);
		result[3][3] = 0;

		return result;
	}

	//decompose
	//*********
	template <class T>
	void decomposeTRS(const matrix<4, 4, T>& mat, vector<3, T>& translation, quaternion<T>& rotation, vector<3, T>& scaleVec)
	{
		//Translation
		translation = mat[3].xyz;

		//Scale
		scaleVec.x = length(mat[0].xyz);
		scaleVec.y = length(mat[1].xyz);
		scaleVec.z = length(mat[2].xyz);

		//Rotation
		matrix<3, 3, T> rotMat(uninitialized);
		rotMat[0] = mat[0].xyz / scaleVec.x;
		rotMat[1] = mat[1].xyz / scaleVec.y;
		rotMat[2] = mat[2].xyz / scaleVec.z;
		rotation = quat(rotMat);
	}

} // namespace etm