#include "Random.h"
#include <limits>


namespace et {
namespace math {


//==========
// Random
//==========


// functionality
/////////////////

//------------------------------
// Random::c-tor
//
Random::Random(uint32 const seed)
	: m_MT(seed)
{ }


// functionality
/////////////////

//------------------------------
// Random::SetSeed
//
void Random::SetSeed(uint32 const s)
{
	m_MT.seed(s);
}


// accessors
/////////////////

//------------------------------
// Random::RandFloat
//
float Random::RandFloat() 
{
	return std::uniform_real_distribution<float>(0.f, 1.f)(m_MT);
}

//------------------------------
// Random::RandFloat
//
float Random::RandFloat(float const min, float const max) 
{
	return std::uniform_real_distribution<float>(min, max)(m_MT);
}


//------------------------------
// Random::RandUint
//
uint32 Random::RandUint() 
{
	return std::uniform_int_distribution<uint32>(0u, std::numeric_limits<uint32>::max())(m_MT);
}

//------------------------------
// Random::RandUint
//
uint32 Random::RandUint(uint32 const max) 
{
	return std::uniform_int_distribution<uint32>(0u, max)(m_MT);
}

//------------------------------
// Random::RandUint
//
uint32 Random::RandUint(uint32 const min, uint32 const max) 
{
	return std::uniform_int_distribution<uint32>(min, max)(m_MT);
}


//------------------------------
// Random::RandInt
//
int32 Random::RandInt() 
{
	return std::uniform_int_distribution<int32>(0u, std::numeric_limits<int32>::max())(m_MT);
}

//------------------------------
// Random::RandInt
//
int32 Random::RandInt(int32 const max) 
{
	return std::uniform_int_distribution<int32>(0u, max)(m_MT);
}

//------------------------------
// Random::RandInt
//
int32 Random::RandInt(int32 const min, int32 const max) 
{
	return std::uniform_int_distribution<int32>(min, max)(m_MT);
}


} // namespace math
} // namespace et
