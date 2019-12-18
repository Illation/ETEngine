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


// create from 'real' components
template<typename TComponentType>
RawComponentData MakeRawComponent(TComponentType& comp) { return RawComponentData(TComponentType::GetTypeIndex(), &comp); }


} // namespace framework
