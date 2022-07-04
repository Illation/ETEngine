#include "stdafx.h"
#include "GlobalRandom.h"


namespace et {
namespace core {


//--------------------
// GetGlobalRandom
//
// Globally uniform random engine ensures consistency
//
math::Random& GetGlobalRandom()
{
	static math::Random s_Random(static_cast<uint32>(HighResTime::Now().SinceEpoch().NanoSeconds()));
	return s_Random;
}


} // namespace core
} // namespace et
