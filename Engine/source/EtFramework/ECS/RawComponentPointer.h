#pragma once
#include "ComponentRegistry.h"


namespace framework {


//------------------
// RawComponentPtr
//
// Type erased component data
// - #note: this does not copy the componentData, it just points to it, so the component data must be kept alive until after this was used (to copy)
//
struct RawComponentPtr final
{
	RawComponentPtr(T_CompTypeIdx const idx, void* const componentData) : typeIdx(idx), data(componentData) {}

	T_CompTypeIdx typeIdx = INVALID_COMP_TYPE_IDX;
	void* data = nullptr;
};


// create from 'real' components
template<typename TComponentType>
RawComponentPtr MakeRawComponent(TComponentType& comp) { return RawComponentPtr(TComponentType::GetTypeIndex(), &comp); }


} // namespace framework
