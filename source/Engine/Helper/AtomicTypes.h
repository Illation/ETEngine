#pragma once

//#include <stdint.h>
//will need to be updated later, as glm already defines those types and glm is used in global namespace -.-
#ifndef GLM_FORCE_LEFT_HANDED
	#define GLM_FORCE_LEFT_HANDED
#endif
#include <glm/glm.hpp>
#include <cassert>

//integers
//typedef glm::int8	int8;
//typedef glm::int16	int16;
//typedef glm::int32	int32;
//typedef glm::int64	int64;
//
//typedef glm::uint8	uint8;
//typedef glm::uint16	uint16;
//typedef glm::uint32	uint32;
//typedef glm::uint64	uint64;

//float
static_assert (sizeof(float) == 4, "compiler uses unexpected size for single precision floats, 32 bit assumed");
typedef float float32;

static_assert (sizeof(double) == 8, "compiler uses unexpected size for double precision floats, 64 bit assumed");
typedef double float64;