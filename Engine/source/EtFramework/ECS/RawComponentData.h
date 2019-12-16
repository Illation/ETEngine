#pragma once
#include "ComponentRegistry.h"


namespace framework {


//------------------
// RawComponentData
//
// Type erased component data 
//
struct RawComponentData final
{
	RawComponentData(T_CompTypeIdx const idx, void* const componentData) : typeIdx(idx), data(componentData) {}

	T_CompTypeIdx const typeIdx = INVALID_COMP_TYPE_IDX;
	void* const data = nullptr;
};


} // namespace framework
