#pragma once

//#include "../Helper/AtomicTypes.hpp"
#include "Vector.hpp"

#include <initializer_list>
#include <array>
#include <assert.h>
#include <stdint.h>

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

	template <uint8 m, uint8 n, class T>
	struct matrix
	{
	public:
		//members
		union
		{
			T data[m][n];
			struct
			{
				etm::vector<n, T> rows[m];
			};
		};

		static uint8 Rows() { return m; }
		static uint8 Collumns() { return n; }
		typedef T value_type;

		//Constructors
		//Identity default constructor
		matrix()
		{
			for (uint8 rowIdx = 0; rowIdx < m; ++rowIdx)
			{
				for (uint8 colIdx = 0; colIdx < n; ++colIdx)
				{
					data[rowIdx][colIdx] = (rowIdx == colIdx) ? static_cast<T>(1) : static_cast<T>(0);
				}
			}
		}
		explicit matrix( etm::ctor )
		{
			//uninitialized constructor
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
		matrix(const etm::vector<n, T> rowList[m])
		{
			for (uint8 rowIdx = 0; rowIdx < m; ++rowIdx)
			{
				rows[rowIdx] = rowList[rowIdx];
			}
		}

		//operators
		etm::vector<n, T> operator[]( const uint8 rowIdx )const
		{
			assert( rowIdx >= 0 && rowIdx < m );
			return rows[rowIdx];
		}
		etm::vector<n, T>& operator[](const uint8 rowIdx)
		{
			assert(rowIdx >= 0 && rowIdx < m);
			return rows[rowIdx];
		}
	};

	template <class T>
	matrix<3, 3, T> CreateFromMat4(const matrix<4, 4, T> &lhs)
	{
		matrix<3, 3, T> ret( uninitialized );
		ret.rows[0] = lhs.rows[0].xyz;
		ret.rows[1] = lhs.rows[1].xyz;
		ret.rows[2] = lhs.rows[2].xyz;
		return ret;
	}
	template <class T>
	matrix<4, 4, T> CreateFromMat3( const matrix<3, 3, T> &lhs )
	{
		matrix<4, 4, T> ret( uninitialized );
		ret.rows[0] = vector<4, T>( lhs.rows[0], 0);
		ret.rows[1] = vector<4, T>( lhs.rows[1], 0);
		ret.rows[2] = vector<4, T>( lhs.rows[2], 0);
		ret.rows[3] = vector<4, T>(0, 0, 0, 1);
		return ret;
	}
	template <class T>
	matrix<4, 4, T> DiscardW( const matrix<4, 4, T> &lhs )
	{
		matrix<4, 4, T> ret( uninitialized );
		ret.rows[0] = vector<4, T>( lhs.rows[0].xyz, 0 );
		ret.rows[1] = vector<4, T>( lhs.rows[1].xyz, 0 );
		ret.rows[2] = vector<4, T>( lhs.rows[2].xyz, 0 );
		ret.rows[3] = vector<4, T>( 0, 0, 0, 1 );
		return ret;
	}

	template <uint8 m, uint8 n, class T>
	inline bool nearEqualsM( matrix<m, n, T> lhs, matrix<m, n, T> rhs, T epsilon = ETM_DEFAULT_EPSILON )
	{
		for(uint8 i = 0; i < m; ++i)
		{
			if(!nearEqualsV( lhs[i], rhs[i], epsilon )) return false;
		}
		return true;
	}

	//shorthands
	typedef matrix<2, 2, float>  mat2;
	typedef matrix<3, 3, float>  mat3;
	typedef matrix<4, 4, float>  mat4;
	typedef matrix<2, 2, double> dmat2;
	typedef matrix<3, 3, double> dmat3;
	typedef matrix<4, 4, double> dmat4;

	//matrix operators
	//****************
	template <uint8 m, uint8 n, class T>
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
	template <uint8 m, uint8 n, class T>
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
	template <uint8 m, uint8 n, class T>
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
					value += lhs[row][j] * rhs[j][col];
				}
				result[row][col] = value;
			}
		}
		return result;
	}
	//vector - matrix multiplication - row major convention -> vectors are 3x1 matrices and therefore on the right side
	template <uint8 m, uint8 n, class T>
	vector<m, T> operator*(matrix<m, n, T>& lhs, vector<n, T>& rhs)
	{
		vector<m, T> result;
		for (uint8 rowIdx = 0; rowIdx < m; ++rowIdx)
		{
			result[rowIdx] = etm::dot(lhs[rowIdx], rhs);
		}
		return result;
	}
	//scalar - matrix multiplication
	template <uint8 m, uint8 n, class T>
	matrix<m, n, T> operator*( matrix<m, n, T>& lhs, const T rhs )
	{
		matrix<m, n, T> result;
		for(uint8 rowIdx = 0; rowIdx < m; ++rowIdx)
		{
			result[rowIdx] = lhs[rowIdx] * rhs;
		}
		return result;
	}

	//operations
	//**********

	template <unsigned int m, unsigned int n, class T>
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

	//special cases

	//determinant
	template <class T>
	inline T determinant(const matrix<2, 2, T>& mat)
	{
		return mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1];
	}
	template <class T>
	inline T determinant(const matrix<3, 3, T>& mat)
	{
		return
			+ mat[0][0] * (mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2])
			- mat[1][0] * (mat[0][1] * mat[2][2] - mat[2][1] * mat[0][2])
			+ mat[2][0] * (mat[0][1] * mat[1][2] - mat[1][1] * mat[0][2]);
	}
	template <class T>
	inline T determinant(const matrix<4, 4, T>& mat)
	{
		T subFactor00 = mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3];
		T subFactor01 = mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3];
		T subFactor02 = mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2];
		T subFactor03 = mat[2][0] * mat[3][3] - mat[3][0] * mat[2][3];
		T subFactor04 = mat[2][0] * mat[3][2] - mat[3][0] * mat[2][2];
		T subFactor05 = mat[2][0] * mat[3][1] - mat[3][0] * mat[2][1];

		vector<4, T> detCof(
			+(mat[1][1] * subFactor00 - mat[1][2] * subFactor01 + mat[1][3] * subFactor02),
			-(mat[1][0] * subFactor00 - mat[1][2] * subFactor03 + mat[1][3] * subFactor04),
			+(mat[1][0] * subFactor01 - mat[1][1] * subFactor03 + mat[1][3] * subFactor05),
			-(mat[1][0] * subFactor02 - mat[1][1] * subFactor04 + mat[1][2] * subFactor05));

		return
			mat[0][0] * detCof[0] + mat[0][1] * detCof[1] +
			mat[0][2] * detCof[2] + mat[0][3] * detCof[3];
	}

	//inverse -- not 100% safe as we don't check the determinant if the determinant is zero all values will be +infinity
	template <class T>
	inline matrix<2, 2, T> inverse(const matrix<2, 2, T>& mat)
	{
		T detFrac = static_cast<T>(1) / etm::determinant(mat);

		matrix<2, 2, T> result( {
			+mat[1][1] * detFrac,
			-mat[0][1] * detFrac,
			-mat[1][0] * detFrac,
			+mat[0][0] * detFrac } );

		return result;
	}
	template <class T>
	inline matrix<3, 3, T> inverse(const matrix<3, 3, T>& mat)
	{
		T detFrac = static_cast<T>(1) / etm::determinant(mat);

		matrix<3, 3, T> result( {
			+(mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1]) * detFrac,
			-(mat[0][1] * mat[2][2] - mat[0][2] * mat[2][1]) * detFrac,
			+(mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1]) * detFrac,
			-(mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0]) * detFrac,
			+(mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0]) * detFrac,
			-(mat[0][0] * mat[1][2] - mat[0][2] * mat[1][0]) * detFrac,
			+(mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]) * detFrac,
			-(mat[0][0] * mat[2][1] - mat[0][1] * mat[2][0]) * detFrac,
			+(mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0]) * detFrac } );

		return result;
	}
	template <class T>
	inline matrix<4, 4, T> inverse(const matrix<4, 4, T>& mat)
	{
		T coef00 = mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3];
		T coef02 = mat[1][2] * mat[3][3] - mat[3][2] * mat[1][3];
		T coef03 = mat[1][2] * mat[2][3] - mat[2][2] * mat[1][3];

		T coef04 = mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3];
		T coef06 = mat[1][1] * mat[3][3] - mat[3][1] * mat[1][3];
		T coef07 = mat[1][1] * mat[2][3] - mat[2][1] * mat[1][3];

		T coef08 = mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2];
		T coef10 = mat[1][1] * mat[3][2] - mat[3][1] * mat[1][2];
		T coef11 = mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2];

		T coef12 = mat[2][0] * mat[3][3] - mat[3][0] * mat[2][3];
		T coef14 = mat[1][0] * mat[3][3] - mat[3][0] * mat[1][3];
		T coef15 = mat[1][0] * mat[2][3] - mat[2][0] * mat[1][3];

		T coef16 = mat[2][0] * mat[3][2] - mat[3][0] * mat[2][2];
		T coef18 = mat[1][0] * mat[3][2] - mat[3][0] * mat[1][2];
		T coef19 = mat[1][0] * mat[2][2] - mat[2][0] * mat[1][2];

		T coef20 = mat[2][0] * mat[3][1] - mat[3][0] * mat[2][1];
		T coef22 = mat[1][0] * mat[3][1] - mat[3][0] * mat[1][1];
		T coef23 = mat[1][0] * mat[2][1] - mat[2][0] * mat[1][1];

		vector<4, T> fac0(coef00, coef00, coef02, coef03);
		vector<4, T> fac1(coef04, coef04, coef06, coef07);
		vector<4, T> fac2(coef08, coef08, coef10, coef11);
		vector<4, T> fac3(coef12, coef12, coef14, coef15);
		vector<4, T> fac4(coef16, coef16, coef18, coef19);
		vector<4, T> fac5(coef20, coef20, coef22, coef23);

		vector<4, T> vec0(mat[1][0], mat[0][0], mat[0][0], mat[0][0]);
		vector<4, T> vec1(mat[1][1], mat[0][1], mat[0][1], mat[0][1]);
		vector<4, T> vec2(mat[1][2], mat[0][2], mat[0][2], mat[0][2]);
		vector<4, T> vec3(mat[1][3], mat[0][3], mat[0][3], mat[0][3]);

		vector<4, T> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
		vector<4, T> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
		vector<4, T> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
		vector<4, T> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

		vector<4, T> signA(+1, -1, +1, -1);
		vector<4, T> signB(-1, +1, -1, +1);
		matrix<4, 4, T> result( new vector<4, T>[4]{ inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB } );

		vector<4, T> row0(result[0][0], result[1][0], result[2][0], result[3][0]);

		vector<4, T> dot0(mat[0] * row0);
		T dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

		T detFrac = static_cast<T>(1) / dot1;

		return result * detFrac;
	}

}//namespace etm