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


} // namespace detail


//-------------------------------
// GetHash
//
// Get a hash from a string
//
constexpr T_Hash GetHash(const std::string &str)
{
	return detail::fnv1a_32(str.c_str(), str.size());
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
	return detail::fnv1a_32(s, count);
}