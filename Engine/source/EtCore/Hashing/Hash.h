#pragma once
#include <EtCore/Util/AtomicTypes.h>
#include <cstring>
#include <string>

//as seen here https://gist.github.com/Lee-R/3839813


namespace et {


// typedefs
typedef uint32 T_Hash;


// implementation
namespace detail
{

	constexpr T_Hash fnv1a_32(char const* const s, size_t const count);
	constexpr T_Hash hash_gen(char const* const s, size_t const count);

} // namespace detail


constexpr T_Hash GetHash(std::string const& str);
constexpr T_Hash GetDataHash(uint8 const* const data, size_t const count);

inline constexpr T_Hash operator"" _hash(char const* const s, size_t const count);


} // namespace et


#include "Hash.inl"
