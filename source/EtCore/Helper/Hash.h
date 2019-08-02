#pragma once
#include "AtomicTypes.h"
#include <cstring>
#include <string>

//as seen here https://gist.github.com/Lee-R/3839813


// typedefs
typedef uint32 T_Hash;


// implementation
namespace detail
{

	constexpr T_Hash fnv1a_32(char const* s, size_t count);
	constexpr T_Hash hash_gen(char const* s, size_t count);

} // namespace detail


constexpr T_Hash GetHash(const std::string &str);

inline constexpr T_Hash operator"" _hash(char const* s, size_t count);


#include "Hash.inl"