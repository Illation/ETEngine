#pragma once


namespace et {


//===================
// Hash
//===================


namespace detail {


//-------------------------------
// fnv1a_32
//
// FNV-1a 32bit hashing algorithm.
//
constexpr T_Hash detail::fnv1a_32(char const* const s, size_t const count)
{
	return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

//-------------------------------
// hash_gen
//
// Wraps around FNV-1a 32 in order to ensure an empty string generates a hash of 0
//
constexpr T_Hash hash_gen(char const* const s, size_t const count)
{
	return (count > 0u) ? fnv1a_32(s, count) : 0u;
}


} // namespace detail


//-------------------------------
// GetHash
//
// Get a hash from a string
//
constexpr T_Hash GetHash(std::string const& str)
{
	return detail::hash_gen(str.c_str(), str.size());
}

//-------------------------------
// GetDataHash
//
// Get a hash from a byte array
//
constexpr T_Hash GetDataHash(uint8 const* const data, size_t const count)
{
	return detail::hash_gen(reinterpret_cast<char const*>(data), count);
}

//-------------------------------
// operator"" _hash
//
// Get a hash from a char literal, example: 
//
// "this will return a T_Hash"_hash
//
inline constexpr T_Hash operator"" _hash(char const* const s, size_t const count)
{
	return detail::hash_gen(s, count);
}


} // namespace et
