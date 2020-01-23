#pragma once
#include <random>

#include "MathUtil.h"


namespace et {
namespace math {


//---------------------
// Random
//
// Wrapper for whatever random algorithm is chosen as implementation - for now we'll use mersenne twister
//
class Random final
{
	// construct destruct
	//--------------------
public:
	Random(uint32 const seed);

	// functionality
	//---------------
	void SetSeed(uint32 const seed);

	// accessors
	//-----------
	float RandFloat(); // [0, 1]
	float RandFloat(float const min, float const max);

	uint32 RandUint(); // [0, largest possible uint32]
	uint32 RandUint(uint32 const max); // [0, max]
	uint32 RandUint(uint32 const min, uint32 const max);

	int32 RandInt(); // [0, largest possible int32]
	int32 RandInt(int32 const max); // [0, max]
	int32 RandInt(int32 const min, int32 const max);

	// Data
	///////

private:
	std::mt19937 m_MT;
};


} // namespace math
} // namespace et
