#pragma once
#include "AtomicTypes.h"
#include <cstring>
#include <string>

//as seen here https://gist.github.com/Lee-R/3839813

namespace detail
{
	// FNV-1a 32bit hashing algorithm.
	inline constexpr uint32 fnv1a_32(char const* s, std::size_t count)
	{
		return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
	}
}    // namespace detail

inline constexpr uint32 operator"" _hash(char const* s, std::size_t count)
{
	return detail::fnv1a_32(s, count);
}

uint32 FnvHash(const std::string &str);