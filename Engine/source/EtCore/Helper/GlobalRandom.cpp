#include "stdafx.h"
#include "GlobalRandom.h"


namespace core {


//--------------------
// GetGlobalRandom
//
// Globally uniform random engine ensures consistency
//
etm::Random& GetGlobalRandom()
{
	static etm::Random s_Random(static_cast<uint32>(ContextManager::GetInstance()->GetActiveContext()->time->SystemTimestamp()));
	return s_Random;
}


} // namespace core

