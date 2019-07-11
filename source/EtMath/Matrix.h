#pragma once
#pragma warning(disable : 4201) //nameless struct union - used in math library

#include "Vector.h"

namespace etm
{
	//Generic matrix
	//**************

	// m = number of rows
	// n = number of collumns
	// T = data type
	// Matrices use row vector notation -> access using row index, then column index
	// Nonetheless matrices are multiplied with vectors using column major convention for opengl compatibility of transformations

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
				vector<n, T> rows[m];
			};
		};

		static uint8 Rows() { return m; }
		static uint8 Collumns() { return n; }
		typedef T value_type;

		//Constructors
		//Identity default constructor
		matrix();
		explicit matrix(etm::ctor) {} //uninitialized constructor
		matrix(const std::initializer_list<T> args);
		matrix(const etm::vector<n, T> rowList[m]);

		//operators
		vector<n, T> operator[](const uint8 rowIdx) const;
		vector<n, T>& operator[](const uint8 rowIdx);
	};

	template <class T>
	matrix<3, 3, T> CreateFromMat4(const matrix<4, 4, T> &lhs);

	template <class T>
	matrix<4, 4, T> CreateFromMat3( const matrix<3, 3, T> &lhs );

	template <class T>
	matrix<4, 4, T> DiscardW( const matrix<4, 4, T> &lhs );

	template <uint8 m, uint8 n, class T>
	bool nearEqualsM( const matrix<m, n, T> &lhs, const matrix<m, n, T> &rhs, const T epsilon = ETM_DEFAULT_EPSILON_T );

	template <uint8 m, uint8 n, class T>
	bool operator==(const matrix<m, n, T>& lhs, const matrix<m, n, T>& rhs);

	//matrix operators
	//****************
	template <uint8 m, uint8 n, class T>
	matrix<m, n, T> operator+(const matrix<m, n, T>& lhs, const matrix<m, n, T>& rhs);

	template <uint8 m, uint8 n, class T>
	matrix<m, n, T> operator-(const matrix<m, n, T>& lhs, const matrix<m, n, T>& rhs);

	//multiplication: lhs rows * rhs cols
	template <uint8 m, uint8 n, class T>
	matrix<m, m, T> operator*(const matrix<m, n, T>& lhs, const matrix<n, m, T>& rhs);

	//vector - matrix multiplication - col major convention -> vectors are 3x1 matrices and therefore on the right side
	template <uint8 m, uint8 n, class T>
	vector<m, T> operator*(const matrix<m, n, T>& lhs, const vector<m, T>& rhs);

	//scalar - matrix multiplication
	template <uint8 m, uint8 n, class T>
	matrix<m, n, T> operator*( const matrix<m, n, T>& lhs, const T rhs );

	//operations
	//**********

	template <unsigned int m, unsigned int n, class T>
	matrix<m, n, T> transpose(const matrix<m, n, T>& mat);

	//special cases

	//determinant
	template <class T>
	T determinant(const matrix<2, 2, T>& mat);

	template <class T>
	T determinant(const matrix<3, 3, T>& mat);

	template <class T>
	T determinant(const matrix<4, 4, T>& mat);

	//inverse -- not 100% safe as we don't check the determinant if the determinant is zero all values will be +infinity
	template <class T>
	matrix<2, 2, T> inverse(const matrix<2, 2, T>& mat);

	template <class T>
	matrix<3, 3, T> inverse(const matrix<3, 3, T>& mat);

	template <class T>
	matrix<4, 4, T> inverse(const matrix<4, 4, T>& mat);

	//access to array for the graphics api
	template<unsigned int m, unsigned int n, typename T>
	T const* valuePtr( matrix<m, n, T> const& mat );

	template<unsigned int m, unsigned int n, typename T>
	T* valuePtr( matrix<m, n, T>& mat );

}//namespace etm

//shorthands
typedef etm::matrix<2, 2, float>  mat2;
typedef etm::matrix<3, 3, float>  mat3;
typedef etm::matrix<4, 4, float>  mat4;
typedef etm::matrix<2, 2, double> dmat2;
typedef etm::matrix<3, 3, double> dmat3;
typedef etm::matrix<4, 4, double> dmat4;

#include "Matrix.inl"