#pragma once

#include <stdint.h>
#include <cassert>


namespace et {


// Integers
typedef std::int8_t		int8;
typedef std::int16_t	int16;
typedef std::int32_t	int32;
typedef std::int64_t	int64;

typedef std::uint8_t	uint8;
typedef std::uint16_t	uint16;
typedef std::uint32_t	uint32;
typedef std::uint64_t	uint64;

// characters
typedef char32_t char32;

// float
static_assert (sizeof(float) == 4, "compiler uses unexpected size for single precision floats, 32 bit assumed");
typedef float float32;

static_assert (sizeof(double) == 8, "compiler uses unexpected size for double precision floats, 64 bit assumed");
typedef double float64;


} // namespace et
