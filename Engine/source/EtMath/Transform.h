#pragma once

#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"


//ETEngine math
namespace etm
{

	//scaling
	//*******
	template <uint8 n, class T>
	void scale(matrix<n, n, T>& result, const vector<n, T> &scaleVec );

	template <uint8 n, class T>
	matrix<n, n, T> scale(const vector<n, T>& scaleVec);

	//specialization for 3 dimensions
	template <class T>
	matrix<4, 4, T> scale(const vector<3, T> &scaleVec );

	template <class T>
	void scale(matrix<4, 4, T>& result, const vector<3, T>& scaleVec );

	//rotation
	//********
	// #todo support euler rotations - for now can be done from a quaternion
	// #todo support 2D rotations
	template <class T>
	matrix<4, 4, T> rotate(const quaternion<T>& rotation);

	template <class T>
	void rotate(matrix<4, 4, T> &result, const quaternion<T>& rotation);

	template <class T>
	matrix<4, 4, T> rotate(const vector<3, T>& axis, const T& angle);

	template <class T>
	void rotate(matrix<4, 4, T> &result, const vector<3, T>& axis, const T& angle);

	//translation
	//***********
	template <class T>
	matrix<4, 4, T> translate(const vector<3, T>& translation);

	template <class T>
	void translate(matrix<4, 4, T> &result, const vector<3, T>& translation);

	//look at
	//*******
	template <class T>
	matrix<4, 4, T> lookAt(const vector<3, T>& position, const vector<3, T>& target, const vector<3, T>& worldUp);

	//projection
	//**********
	template <class T>
	matrix<4, 4, T> orthographic(const T& left, const T& right, const T& top, const T& bottom, const T& nearZ, const T& farZ );

	template <class T>
	matrix<4, 4, T> perspective(const T& fov, const T& aspect, const T& nearZ, const T& farZ);

	//decompose
	//*********
	template <class T>
	void decomposeTRS(const matrix<4, 4, T>& mat, vector<3, T>& translation, quaternion<T>& rotation, vector<3, T>& scaleVec);

	template <class T>
	vector<3, T> decomposeScale(matrix<4, 4, T> const& mat);

} // namespace etm

#include "Transform.inl"