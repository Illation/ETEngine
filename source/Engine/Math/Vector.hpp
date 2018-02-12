#pragma once
#pragma warning(disable : 4201) //nameless struct union - used in math library

#include "MathUtil.hpp"

#include <initializer_list>
#include <array>
#include <rttr/registration>

//ETEngine math
namespace etm
{
	enum ctor { uninitialized };

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
	public:
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
			assert(args.size() <= n);
			int32 index = 0;
			for (auto begin = args.begin(); begin != args.end(); ++begin)
			{
				data.at(index++) = *begin;
			}
		}

		//operators
		T operator[] (const uint8 index) const
		{
			assert(index >= 0 && index < n);
			return data.at(index);
		}
		T& operator[] (const uint8 index)
		{
			assert(index >= 0 && index < n);
			return data.at(index);
		}
		vector<n, T> operator-() const;

		//string conversion
		std::string ToString() const;
		operator std::string() const { return ToString(); }

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
		T operator[] (const uint8 index) const
		{
			assert(index >= 0 && index < 2);
			return data.at(index);
		}
		T& operator[] (const uint8 index)
		{
			assert(index >= 0 && index < 2);
			return data.at(index);
		}
		vector<2, T> operator-() const;

		//string conversion
		std::string ToString() const;
		operator std::string() const { return ToString(); }
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

		static vector<3, T> ZERO;
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
		T operator[] (const uint8 index) const
		{
			assert(index >= 0 && index < 3);
			return data.at(index);
		}
		T& operator[] (const uint8 index)
		{
			assert(index >= 0 && index < 3);
			return data.at(index);
		}
		vector<3, T> operator-() const;

		//string conversion
		std::string ToString() const;
		operator std::string() const { return ToString(); }
	};

	template<typename T> vector<3, T> vector<3, T>::ZERO = vector<3, T>(0.0, 0.0, 0.0);
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
		T operator[] (const uint8 index) const
		{
			assert(index >= 0 && index < 4);
			return data.at(index);
		}
		T& operator[] (const uint8 index)
		{
			assert(index >= 0 && index < 4);
			return data.at(index);
		}
		vector<4, T> operator-() const;

		//string conversion
		std::string ToString() const;
		operator std::string() const { return ToString(); }
	};


	//operators
	//*********
	template <uint8 n, class T>
	std::string etm::vector<n, T>::ToString() const
	{
		std::string ret = "[";
		for(uint8 i = 0; i < data.size(); ++i)
			ret += (i == data.size() - 1) ? (data[i]) : (data[i]+", ");
		return ret + "]";
	}
	template <uint8 n, class T>
	std::ostream& operator<<( std::ostream& os, const etm::vector<n, T>& vec)
	{
		return os << vec.ToString();
	}

	template <class T>
	std::string etm::vector<2, T>::ToString() const
	{
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + "]";
	}
	template <class T>
	std::ostream& operator<<( std::ostream& os, const etm::vector<2, T>& vec)
	{
		return os << vec.ToString();
	}

	template <class T>
	std::string etm::vector<3, T>::ToString() const
	{
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";
	}
	template <class T>
	std::ostream& operator<<( std::ostream& os, const etm::vector<3, T>& vec)
	{
		return os << vec.ToString();
	}

	template <class T>
	std::string etm::vector<4, T>::ToString() const
	{
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) 
			+ ", " + std::to_string(z) + ", " + std::to_string(w) + "]";
	}
	template <class T>
	std::ostream& operator<<( std::ostream& os, const etm::vector<4, T>& vec)
	{
		return os << vec.ToString();
	}

	//negate
	template <uint8 n, class T>
	inline vector<n, T> vector<n, T>::operator-() const
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i)
		{
			result[i] = -data[i];
		}
		return result;
	}
	template <class T>
	inline vector<2, T> vector<2, T>::operator-() const
	{
		return{ -x, -y };
	}
	template <class T>
	inline vector<3, T> vector<3, T>::operator-() const
	{
		return{ -x, -y, -z };
	}
	template <class T>
	inline vector<4, T> vector<4, T>::operator-() const
	{
		return{ -x, -y, -z, -w };
	}

	// addition
	template <uint8 n, class T>
	inline vector<n, T> operator+(const vector<n, T> &lhs, const T scalar)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] + scalar;
		}
		return result;
	}
	template <uint8 n, class T>
	inline vector<n, T> operator+(const T scalar, const vector<n, T> &rhs)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i)
			result[i] = lhs[i] + scalar;
		return result;
	}
	template <uint8 n, class T>
	inline vector<n, T> operator+(const vector<n, T> &lhs, const vector<n, T> &rhs)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i)
			result[i] = lhs[i] + rhs[i];
		return result;
	}

	// subtraction
	template <uint8 n, class T>
	inline vector<n, T> operator-(const vector<n, T> &lhs, const T scalar)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] - scalar;
		}
		return result;
	}
	template <uint8 n, class T>
	inline vector<n, T> operator-(const vector<n, T> &lhs, const vector<n, T> &rhs)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] - rhs[i];
		}
		return result;
	}

	// multiplication
	template <uint8 n, class T>
	inline vector<n, T> operator*(const vector<n, T> lhs, const T &scalar)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] * scalar;
		}
		return result;
	}
	template <uint8 n, class T>
	inline vector<n, T> operator*(const T scalar, const vector<n, T> &lhs)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] * scalar;
		}
		return result;
	}
	template <uint8 n, class T>
	inline vector<n, T> operator*(const vector<n, T> &lhs, const vector<n, T> &rhs) // hadamard product
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] * rhs[i];
		}
		return result;
	}

	//division
	template <uint8 n, class T>
	inline vector<n, T> operator/(const vector<n, T> &lhs, const T scalar)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] / scalar;
		}
		return result;
	}
	template <uint8 n, class T>
	inline vector<n, T> operator/(const T scalar, const vector<n, T> &rhs)
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i)
		{
			result[i] = scalar / rhs[i];
		}
		return result;
	}
	template <uint8 n, class T>
	inline vector<n, T> operator/(const vector<n, T> &lhs, const vector<n, T> &rhs) //hadamard product
	{
		vector<n, T> result;
		for (uint8 i = 0; i < n; ++i) 
		{
			result[i] = lhs[i] / rhs[i];
		}
		return result;
	}

	//operations
	//**********
	template <uint8 n, class T>
	inline bool nearEqualsV(const vector<n, T> &lhs, const vector<n, T> &rhs, const T epsilon = ETM_DEFAULT_EPSILON_T )
	{
		for (uint8 i = 0; i < n; ++i)
		{
			if (!nearEquals(lhs[i], rhs[i], epsilon)) return false;
		}
		return true;
	}
	template <uint8 n, class T>
	inline bool isZero(const vector<n, T> &lhs, const T epsilon = static_cast<T>(0))
	{
		return nearEqualsV(lhs, vector<n, T>(0), epsilon);
	}
	template <uint8 n, class T>
	inline bool operator==(const vector<n, T> &lhs, const vector<n, T> &rhs)
	{
		return nearEqualsV(lhs, rhs);
	}

	template <uint8 n, class T>
	inline T dot(const vector<n, T> &lhs, const vector<n, T> &rhs)
	{
		T result = {};
		for (uint8 i = 0; i < n; ++i)
			result += lhs[i] * rhs[i];
		return result;
	}

	//this is important so we do template specaializations
	template <class T>
	inline T dot(const vector<2, T> &lhs, const vector<2, T> &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	template <class T>
	inline T dot(const vector<3, T> &lhs, const vector<3, T> &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	template <class T>
	inline T dot(const vector<4, T> &lhs, const vector<4, T> &rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}

	template <uint8 n, class T>
	inline T lengthSquared(const vector<n, T> &vec)
	{
		return etm::dot(vec, vec);
	}
	template <uint8 n, class T>
	inline T length(const vector<n, T> &vec)
	{
		return sqrt(etm::lengthSquared(vec));
	}

	template <uint8 n, class T>
	inline T distance(const vector<n, T> &lhs, const vector<n, T> &rhs)
	{
		return length(lhs - rhs);
	}
	template <uint8 n, class T>
	inline T distanceSquared(const vector<n, T> &lhs, const vector<n, T> &rhs)
	{
		return lengthSquared(lhs - rhs);
	}

	template <uint8 n, class T>
	inline vector<n, T> normalize(const vector<n, T> &vec)
	{
		//assert(!nearEqualsV(vec, vector<n, T>(0), static_cast<T>(0)));
		T len = length(vec);
		return vec / len;
	}

	template <uint8 n, class T>
	inline vector<n, T> pow(const vector<n, T> &vec, T exponent)
	{
		vector<n, T> result = vector<n, T>();
		for (uint8 i = 0; i < n; ++i)
			result[i] = std::pow(vec[i], exponent);
		return result;
	}

	//Vectors need to be prenormalized
	//if input vectors are zero it will generate NaN
	template <uint8 n, class T>
	inline T angleFastUnsigned(const vector<n, T>& lhs, const vector<n, T>& rhs)
	{
		return acos(dot(lhs, rhs));
	}
	template <uint8 n, class T>
	inline T angleSafeUnsigned(const vector<n, T>& lhs, const vector<n, T>& rhs)
	{
		return angleFastUnsigned(normalize(lhs), normalize(rhs));
	}

	//access to array for the graphics api
	template<uint8 n, typename T>
	T const* valuePtr( vector<n, T> const& vec )
	{
		return &(vec.data[0]);
	}
	template<uint8 n, typename T>
	T* valuePtr( vector<n, T>& vec )
	{
		return &(vec.data[0]);
	}

	template<typename T, uint8 n,typename T2>
	inline vector<n, T> vecCast(const vector<n, T2> &vec)
	{
		vector<n, T> ret;
		for (uint8 i = 0; i < n; ++i)
		{
			ret[i] = static_cast<T>(vec[i]);
		}
		return ret;
	}

	//dimension specific operations
	//*****************************
	//vec2
	template<class T>
	inline vector<2, T> perpendicular(const vector<2, T>& vec)
	{
		vector<2, T> result;
		result.x = -vec.y;
		result.y = vec.x;
		return result;
	}
	//Vectors need to be prenormalized
	template<class T>
	inline T angleSigned(const vector<2, T>& lhs, const vector<2, T>& rhs)
	{
		return atan2(rhs.y, rhs.x) - atan2(lhs.y, lhs.x);
	}

	//vec3
	//direction will be left handed (lhs = thumb, rhs = index, result -> middle finger);
	template<class T>
	inline vector<3, T> cross(const vector<3, T>& lhs, const vector<3, T>& rhs)
	{
		vector<3, T> result;

		result.x = lhs.y*rhs.z - lhs.z*rhs.y;
		result.y = lhs.z*rhs.x - lhs.x*rhs.z;
		result.z = lhs.x*rhs.y - lhs.y*rhs.x;

		return result;
	}
	//inputs vectors must be prenormalized, 
	//and, for accurate measurments the outAxis should also be normalized after
	//if input vectors are zero it will generate NaN
	template<class T>
	inline T angleFastAxis(const vector<3, T>& lhs, const vector<3, T>& rhs, vector<3, T> &outAxis)
	{
		outAxis = cross(lhs, rhs);
		return angleFastUnsigned(lhs, rhs);
	}
	template<class T>
	inline T angleSafeAxis(const vector<3, T>& lhs, const vector<3, T>& rhs, vector<3, T> &outAxis)
	{
		vector<3, T> lhsN = normalize(lhs);
		vector<3, T> rhsN = normalize(rhs);
		T result = angleFastAxis(lhsN, rhsN, outAxis);
		outAxis = normalize(outAxis);
		return result;
	}
	
} // namespace etm

//shorthands
typedef etm::vector<2, float>  vec2;
typedef etm::vector<3, float>  vec3;
typedef etm::vector<4, float>  vec4;
typedef etm::vector<2, int32>  ivec2;
typedef etm::vector<3, int32>  ivec3;
typedef etm::vector<4, int32>  ivec4;
typedef etm::vector<2, double> dvec2;
typedef etm::vector<3, double> dvec3;
typedef etm::vector<4, double> dvec4;

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<vec3>("vec3")
		.constructor<>()
		.constructor<const float&>()
		.constructor<const std::initializer_list<float>>()
		.property("x", &vec3::x)
		.property("y", &vec3::y)
		.property("z", &vec3::z);
}