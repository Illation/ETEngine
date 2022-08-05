#pragma once
#include <EtCore/Platform/AtomicTypes.h>
#include <cstring>
#include <string>

//as seen here https://gist.github.com/Lee-R/3839813


namespace et {


// typedefs
typedef uint32 T_Hash;

constexpr T_Hash GetHash(std::string const& str);
constexpr T_Hash GetDataHash(uint8 const* const data, size_t const count);

inline constexpr T_Hash operator"" _hash(char const* const s, size_t const count);


} // namespace et


#include "Hash.inl"
