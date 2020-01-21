#pragma once
#include "ComponentRegistry.h"


namespace et {
namespace fw {


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


namespace detail {

	//----------------------------------
	// EcsCommandBuffer::GenCompPtrList
	//
	// variadic template recursively adds components to a raw component pointer list
	//
	template<typename TComponentType>
	void GenCompPtrList(std::vector<RawComponentPtr>& list, TComponentType& component)
	{
		list.emplace_back(MakeRawComponent(component));
	}

	template<typename TComponentType, typename... Args>
	void GenCompPtrList(std::vector<RawComponentPtr>& list, TComponentType& component1, Args... args)
	{
		list.emplace_back(MakeRawComponent(component1));
		GenCompPtrList(list, args...);
	}

} // namespace detail


} // namespace fw
} // namespace et
