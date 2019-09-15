#pragma once


//===================
// Hash
//===================


namespace detail {


//-------------------------------
// fnv1a_32
//
// FNV-1a 32bit hashing algorithm.
//
constexpr T_Hash detail::fnv1a_32(char const* s, size_t count)
{
	return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

//-------------------------------
// hash_gen
//
// Wraps around FNV-1a 32 in order to ensure an empty string generates a hash of 0
//
constexpr T_Hash hash_gen(char const* s, size_t count)
{
	return (count > 0) ? fnv1a_32(s, count) : 0u;
}

} // namespace detail


//-------------------------------
// GetHash
//
// Get a hash from a string
//
constexpr T_Hash GetHash(const std::string &str)
{
	return detail::hash_gen(str.c_str(), str.size());
}

//-------------------------------
// operator"" _hash
//
// Get a hash from a char literal, example: 
//
// "this will return a T_Hash"_hash
//
inline constexpr T_Hash operator"" _hash(char const* s, size_t count)
{
	return detail::hash_gen(s, count);
}