#pragma once

//#include "../Helper/AtomicTypes.h"
#include "Vector.h"
#include "Matrix.h"


//ETEngine math
namespace etm
{

	//scaling
	//*******
	template <uint8 n, class T>
	inline matrix<n, n, T>& scale(matrix<n, n, T>& result, vector<n, T> scale)
	{
		for (uint8 i = 0; i < n; ++i)
		{
			result[i][i] *= scale[i];
		}
		return result;
	}
	template <uint8 n, class T>
	inline matrix<n, n, T> scale(vector<n, T>& scale)
	{
		matrix<n, n, T> mat;
		return scale(mat, scale);
	}
	//specialization for 3 dimensions
	template <class T>
	inline matrix<4, 4, T> scale(vector<3, T> scale)
	{
		return scale(vector<4, T>(scale, 1));
	}
	template <class T>
	inline matrix<4, 4, T>& scale(matrix<4, 4, T>& result, vector<3, T>& scale)
	{
		return scale(result, vector<4, T>(scale, 1));
	}

	//rotation
	//********
	// #todo support euler rotations
	// #todo support 2D rotations
	template <class T>
	matrix<4, 4, T> rotate(const quaternion<T>& rotation)
	{
		matrix<3, 3, T> result = rotation::ToMatrix();
		return matrix<4, 4, T>(result);
	}
	template <class T>
	matrix<4, 4, T> rotate(const vector<3, T>& axis, const T& angle)
	{
		matrix<4, 4, T> result;

		const float c = cos(angle);
		const float s = sin(angle);
		const float t = 1.0f - c;

		result[0][0] = t*axis.x*axis.x + c;
		result[0][1] = t*axis.x*axis.y - s*axis.z;
		result[0][2] = t*axis.x*axis.z + s*axis.y;

		result[1][0] = t*axis.x*axis.y + s*axis.z;
		result[1][1] = t*axis.y*axis.y + c;
		result[1][2] = t*axis.y*axis.z - s*axis.x;

		result[2][0] = t*axis.x*axis.z - s*axis.y;
		result[2][1] = t*axis.y*axis.z + s*axis.x;
		result[2][2] = t*axis.z*axis.z + c;

		return result;
	}
	template <class T>
	matrix<4, 4, T>& rotate(matrix<4, 4, T> &result, const vector<3, T>& axis, const T& angle)
	{
		matrix<4, 4, T> rot = rotate(axis, angle);
		result = result * rot;
		return result;
	}

	//translation
	//***********
	template <class T>
	matrix<4, 4, T> translate(const vector<3, T>& translation)
	{
		matrix<4, 4, T> mat;
		mat[3] = vector<4, T>(translation, 1.0);
		return mat;
	}
	template <class T>
	matrix<4, 4, T>& translate(matrix<4, 4, T> &result, const vector<3, T>& translation)
	{
		matrix<4, 4, T> mat;
		mat[3] = vector<4, T>(translation, 1.0);

		result = result * mat;
		return result;
	}

	//look at
	//*******
	template <class T>
	matrix<4, 4, T> lookAt(vector<3, T>& position, vector<3, T>& target, vector<3, T>& worldUp)
	{
		matrix<4, 4, T> frame;
		matrix<4, 4, T> translation = translate(-position);

		vec3 forward = normalize(target - position);
		vec3 right = normalize(cross(forward, worldUp));
		vec3 up = cross(right, forward);

		frame[0].xyz = right;
		frame[1].xyz = up;
		frame[2].xyz = -forward;
		//frame = transpose(frame);

		return frame*translation;
	}

	//projection
	//**********
	template <class T>
	matrix<4, 4, T> orthographic(const T& left, const T& right, const T& top, const T& bottom, const T& near, const T& far)
	{
		matrix<4, 4, T> result;

		result[0][0] = static_cast<T>(2) / (right - left);
		result[1][1] = static_cast<T>(2) / (top - bottom);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);

		result[2][2] = static_cast<T>(2) / (far - near);
		result[3][2] = -(far + near) / (far - near);
		
		return result;
	}
	template <class T>
	matrix<4, 4, T> perspective(const T& fov, const T& aspect, const T& near, const T& far)
	{
		matrix<4, 4, T> result;

		T const tanHalfFovy = tan(fovy / static_cast<T>(2));

		result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
		result[1][1] = static_cast<T>(1) / (tanHalfFovy);
		result[2][3] = static_cast<T>(1);

		result[2][2] = (zFar + zNear) / (zFar - zNear);
		result[3][2] = -(static_cast<T>(2) * zFar * zNear) / (zFar - zNear);

		return result;
	}

} // namespace etm