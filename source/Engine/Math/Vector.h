#pragma once

#include <initializer_list>
#include <array>
#include <assert.h>

#include "../Helper/AtomicTypes.h"

//inspired by JoeyDeVries/Cell , g-truc/glm , Game Engine Architecture
//implementing a custom math library for the learning experience, completeness, control and easy build setup
//#todo SIMD support

//ETEngine math
namespace etm
{
	//Generic vector
	//**************

	// n = dimensions
	// T = data type

	// In 3D cartesian coordinate space:
	// Left handed - Y is up

	// Vectors are treated as columns when multiplied with matrices

	template <uint8 n, class T>
	struct vector
	{
		//members
		std::array<T, n> data;

		//constructors
		vector()
		{
			for (uint8 i = 0; i < n; ++i)
			{
				data[i] = 0;
			}
		}
		vector(const T &rhs)
		{
			for (auto& el : data) 
			{
				el = rhs;
			}
		}
		vector(const std::initializer_list<T> args)
		{
			assert(args.size() < n);
			data = args;
		}

		//operators
		T& operator[] (const std::size_t index)
		{
			assert(index >= 0 && index < n);
			return data.at(index);
		}
		vector<n, T> operator-();
	};
	//specializations
	//***************

	//vec2
	template <typename T>
	struct vector<2, T>
	{
		union
		{
			std::array<T, 2> data;
			struct
			{
				T x;
				T y;
			};
		};
		vector()
		{
			data = { 0, 0 };
		}
		vector(const T &rhs)
		{
			data = { rhs, rhs };
		}
		vector(const std::initializer_list<T> args)
		{
			assert(args.size() <= 2);
			int32 index = 0;
			for (auto begin = args.begin(); begin != args.end(); ++begin) 
			{
				data.at(index++) = *begin;
			}
		}
		vector(const T& x, const T& y)
		{
			data = { x, y };
		}
		//operators
		T& operator[] (const std::size_t index)
		{
			assert(index >= 0 && index < 2);
			return data.at(index);
		}
		vector<2, T> operator-();
	};

	//vec3
	template<typename T>
	struct vector<3, T>
	{
		union
		{
			std::array<T, 3> data;
			struct
			{
				T x;
				T y;
				T z;
			};
			
			struct
			{
				vector<2, T> xy;
				T _ignored1;
			};
			struct
			{
				T _ignored1;
				vector<2, T> yz;
			};
		};

		static vector<3, T> UP;
		static vector<3, T> DOWN;
		static vector<3, T> LEFT;
		static vector<3, T> RIGHT;
		static vector<3, T> FORWARD;
		static vector<3, T> BACK;

		vector()
		{
			data = { 0, 0, 0 };
		}
		vector(const T &rhs)
		{
			data = { rhs, rhs, rhs };
		}
		vector(const std::initializer_list<T> args)
		{
			assert(args.size() <= 3);
			int32 index = 0;
			for (auto begin = args.begin(); begin != args.end(); ++begin) 
			{
				data.at(index++) = *begin;
			}
		}
		vector(const T& x, const T& y, const T& z)
		{
			data = { x, y, z };
		}
		vector(const vector<2, T>& vec, const T& z)
		{
			data = { vec.x, vec.y, z };
		}
		vector(const T& x, const vector<2, T>& vec)
		{
			data = { x, vec.x, vec.y };
		}

		//operators
		T& operator[] (const std::size_t index)
		{
			assert(index >= 0 && index < 3);
			return data.at(index);
		}
		vector<3, T> operator-();
	};

	template<typename T> vector<3, T> vector<3, T>::UP = vector<3, T>(0.0, 1.0, 0.0);
	template<typename T> vector<3, T> vector<3, T>::DOWN = vector<3, T>(0.0, -1.0, 0.0);
	template<typename T> vector<3, T> vector<3, T>::LEFT = vector<3, T>(-1.0, 0.0, 0.0);
	template<typename T> vector<3, T> vector<3, T>::RIGHT = vector<3, T>(1.0, 0.0, 0.0);
	template<typename T> vector<3, T> vector<3, T>::FORWARD = vector<3, T>(0.0, 0.0, 1.0);
	template<typename T> vector<3, T> vector<3, T>::BACK = vector<3, T>(0.0, 0.0, -1.0);

	//vec4
	template<typename T>
	struct vector<4, T>
	{
		union
		{
			std::array<T, 4> data;
			struct
			{
				T x;
				T y;
				T z;
				T w;
			};
			struct
			{
				T r;
				T g;
				T b;
				T a;
			};
			struct
			{
				vector<2, T> xy;
				T _ignored1;
				T _ignored2;
			};
			struct
			{
				T _ignored1;
				vector<2, T> yz;
				T _ignored2;
			};
			struct
			{
				T _ignored1;
				T _ignored2;
				vector<2, T> zw;
			};
			struct
			{
				vector<3, T> xyz;
				T _ignored1;
			};
			struct
			{
				vector<3, T> rgb;
				T _ignored1;
			};
		};

		//constructors
		vector()
		{
			data = { 0, 0, 0, 0 };
		}
		vector(const T &rhs)
		{
			data = { rhs, rhs, rhs, rhs };
		}
		vector(const std::initializer_list<T> args)
		{
			assert(args.size() <= 4);
			int index = 0;
			for (auto begin = args.begin(); begin != args.end(); ++begin) 
			{
				data.at(index++) = *begin;
			}
		}
		//with scalars
		vector(const T& x, const T& y, const T& z, const T& w)
		{
			data = { x, y, z, w };
		}
		//with vec2
		vector(const vector<2, T>& xy, const vector<2, T>& zw)
		{
			data = { xy.x, xy.y, zw.x, zw.y };
		}
		vector(const vector<2, T>& xy, const T& z, const T& w)
		{
			data = { xy.x, xy.y, z, w };
		}
		vector(const T& x, const T& y, const vector<2, T>& zw)
		{
			data = { x, y, zw.x, zw.y };
		}
		vector(const T& x, const vector<2, T>& yz, const T& w)
		{
			data = { x, yz.x, yz.y, w };
		}
		//with vec3
		vector(const vector<3, T>& xyz, const T& w)
		{
			data = { xyz.x, xyz.y, xyz.z, w };
		}
		vector(const T& x, const vector<3, T>& yzw)
		{
			data = { x, yzw.x, yzw.y, yzw.z };
		}

		//operators
		T& operator[] (const std::size_t index)
		{
			assert(index >= 0 && index < 4);
			return data.at(index);
		}
		vector<4, T> operator-();
	};

	//shorthands
	typedef vector<2, float>  vec2;
	typedef vector<3, float>  vec3;
	typedef vector<4, float>  vec4;
	typedef vector<2, int>    ivec2;
	typedef vector<3, int>    ivec3;
	typedef vector<4, int>    ivec4;
	typedef vector<2, double> dvec2;
	typedef vector<3, double> dvec3;
	typedef vector<4, double> dvec4;

	//operators
	//*********

	//negate
	template <uint8 n, typename T>
	inline vector<n, T> vector<n, T>::operator-()
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) 
		{
			result[i] = -data[i];
		}
		return result;
	}
	template <typename T>
	inline vector<2, T> vector<2, T>::operator-()
	{
		return{ -x, -y };
	}
	template <typename T>
	inline vector<3, T> vector<3, T>::operator-()
	{
		return{ -x, -y, -z };
	}
	template <typename T>
	inline vector<4, T> vector<4, T>::operator-()
	{
		return{ -x, -y, -z, -w };
	}

	// addition
	template <uint8 n, typename T>
	inline vector<n, T> operator+(vector<n, T> lhs, T scalar)
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] + scalar;
		}
		return result;
	}
	template <uint8 n, typename T>
	inline vector<n, T> operator+(T scalar, vector<n, T> rhs)
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i)
			result[i] = lhs[i] + scalar;
		return result;
	}
	template <uint8 n, typename T>
	inline vector<n, T> operator+(vector<n, T> lhs, vector<n, T> rhs)
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i)
			result[i] = lhs[i] + rhs[i];
		return result;
	}

	// subtraction
	template <uint8 n, typename T>
	inline vector<n, T> operator-(vector<n, T> lhs, T scalar)
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] - scalar;
		}
		return result;
	}
	template <uint8 n, typename T>
	inline vector<n, T> operator-(vector<n, T> lhs, vector<n, T> rhs)
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] - rhs[i];
		}
		return result;
	}

	// multiplication
	template <uint8 n, typename T>
	inline vector<n, T> operator*(vector<n, T> lhs, T scalar)
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] * scalar;
		}
		return result;
	}
	template <uint8 n, typename T>
	inline vector<n, T> operator*(T scalar, vector<n, T> lhs)
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] * scalar;
		}
		return result;
	}
	template <uint8 n, typename T>
	inline vector<n, T> operator*(vector<n, T> lhs, vector<n, T> rhs) // hadamard product
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] * rhs[i];
		}
		return result;
	}

	//division
	template <uint8 n, typename T>
	inline vector<n, T> operator/(vector<n, T> lhs, T scalar)
	{
		vector<n, T> result;
		for (unsigned int i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] / scalar;
		}
		return result;
	}
	template <uint8 n, typename T>
	inline vector<n, T> operator/(vector<n, T> lhs, vector<n, T> rhs) //hadamard product
	{
		vector<n, T> result;
		for (std::size_t i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] / rhs[i];
		}
		return result;
	}

	//operations
	//**********

	template <uint8 n, typename T>
	inline T dot(vector<n, T> lhs, vector<n, T> rhs)
	{
		T result = {};
		for (std::size_t i = 0; i < n; ++i)
			result += lhs[i] * rhs[i];
		return result;
	}

	template <uint8 n, typename T>
	inline T lengthSquared(vector<n, T> vec)
	{
		return etm::dot(vec, vec);
	}
	template <uint8 n, typename T>
	inline T length(vector<n, T> vec)
	{
		return sqrt(etm::lengthSquared(vec));
	}

	template <uint8 n, typename T>
	inline float distance(vector<n, T> lhs, vector<n, T> rhs)
	{
		return length(lhs - rhs);
	}
	template <uint8 n, typename T>
	inline float distanceSquared(vector<n, T> lhs, vector<n, T> rhs)
	{
		return lengthSquared(lhs - rhs);
	}

	template <uint8 n, typename T>
	inline vector<n, T> normalize(vector<n, T> vec)
	{
		T len = length(vec);
		return vec / len;
	}

	//dimension specific operations
	template<typename T>
	inline vector<2, T> perpendicular(const vector<2, T>& vec)
	{
		vector<2, T> result;
		result.x = -vec.y;
		result.y = vec.x;
		return result;
	}
	template<typename T>
	inline vector<3, T> cross(const vector<3, T>& lhs, const vector<3, T>& rhs)
	{
		vector<3, T> result;

		result.x = lhs.y*rhs.z - lhs.z*rhs.y;
		result.y = lhs.z*rhs.x - lhs.x*rhs.z;
		result.z = lhs.x*rhs.y - lhs.y*rhs.x;

		return result;
	}

} // namespace etm