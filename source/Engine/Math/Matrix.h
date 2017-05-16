#pragma once

#include "../Helper/AtomicTypes.h"
#include "Vector.h"

//inspired by JoeyDeVries/Cell , g-truc/glm , Game Engine Architecture
//implementing a custom math library for the learning experience, completeness, control and easy build setup
//#todo SIMD support

namespace etm
{
	//Generic matrix
	//**************

	// m = number of rows
	// n = number of collumns
	// T = data type
	// Matrices use row vector notation -> access using row index, then column index

	// | 0  1  2  3  | Row 0
	// | 4  5  6  7  | Row 1
	// | 8  9  10 11 | Row 2
	// | 12 13 14 15 | Row 3

	template <uint8 m, uint8 n, typename T>
	struct matrix
	{

		union
		{
			T data[m][n];
			struct
			{
				vector<n, T> rows[m];
			};
		};

		//Constructors
		//Identity default constructor
		matrix()
		{
			for (uint8 rowIdx; rowIdx < m; ++rowIdx)
			{
				for (uint8 colIdx; colIdx < n; ++colIdx)
				{
					data[rowIdx][colIdx] = (rowIdx == colIdx) ? 1 : 0;
				}
			}
		}
		matrix(const std::initializer_list<T> args)
		{
			assert(args.size() <= m * n);
			uint8 rowIdx = 0, colIdx = 0;

			for (auto& it : args)
			{
				data[rowIdx][colIdx++] = it;
				if (colIdx >= n)
				{
					++rowIdx;
					colIdx = 0;
				}
			}
		}
		matrix(const &vector<n, T> rowList[m])
			: rows(rowList)
		{
		}

		//operators
		vector<n, T>& operator[](const uint8 rowIdx)
		{
			assert(rowIdx >= 0 && rowIdx < m);
			return rows[rowIdx];
		}
	};

	//shorthands
	typedef matrix<2, 2, float>  mat2;
	typedef matrix<3, 3, float>  mat3;
	typedef matrix<4, 4, float>  mat4;
	typedef matrix<2, 2, double> dmat2;
	typedef matrix<3, 3, double> dmat3;
	typedef matrix<4, 4, double> dmat4;

	//matrix operators
	//****************
	template <uint8 m, uint8 n, typename T>
	matrix<m, n, T> operator+(matrix<m, n, T>& lhs, matrix<m, n, T>& rhs)
	{
		matrix<m, n, T> result;
		for (uint8 col = 0; col < n; ++col)
		{
			for (uint8 row = 0; row < m; ++row)
			{
				result[col][row] = lhs[col][row] + rhs[col][row];
			}
		}
		return result;
	}
	template <uint8 m, uint8 n, typename T>
	matrix<m, n, T> operator-(matrix<m, n, T>& lhs, matrix<m, n, T>& rhs)
	{
		matrix<m, n, T> result;
		for (uint8 col = 0; col < n; ++col)
		{
			for (uint8 row = 0; row < m; ++row)
			{
				result[col][row] = lhs[col][row] - rhs[col][row];
			}
		}
		return result;
	}
	//multiplication: lhs rows * rhs columns
	template <uint8 m, uint8 n, typename T>
	matrix<m, m, T> operator*(matrix<m, n, T>& lhs, matrix<n, m, T>& rhs)
	{
		matrix<m, m, T> result;
		for (uint8 col = 0; col < m; ++col)
		{
			for (uint8 row = 0; row < m; ++row)
			{
				//dot product
				T value = {};
				for (uint8 j = 0; j < n; ++j)
				{
					value += lhs[j][row] * rhs[col][j];
				}
				result[col][row] = value;
			}
		}
		return result;
	}
	//vector - matrix multiplication - row major convention -> vectors are 3x1 matrices and therefore on the right side
	template <uint8 m, uint8 n, typename T>
	vector<m, T> operator*(matrix<m, n, T>& lhs, vector<n, T>& rhs)
	{
		vector<m, T> result;
		for (uint8 rowIdx = 0; rowIdx < m; ++rowIdx)
		{
			result[rowIdx] = etm::dot(lhs[rowIdx], rhs);
		}
		return result;
	}

	//operations
	//**********

	template <unsigned int m, unsigned int n, typename T>
	inline matrix<m, n, T> transpose(matrix<m, n, T>& mat)
	{
		matrix<n, m, T> result;

		for (uint8 colIdx = 0; colIdx < m; ++colIdx)
		{
			for (uint8 rowIdx = 0; rowIdx < n; ++rowIdx)
			{
				result[rowIdx][colIdx] = mat[colIdx][rowIdx];
			}
		}
		return result;
	}


	template <uint8  m, uint8  n, typename T>
	inline matrix<m, n, T> inverse(const matrix<m, n, T>& mat)
	{
		matrix<m, n, T> result;
		// #todo: matrix inverse
		return result;
	}

}//namespace etm