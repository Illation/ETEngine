#pragma once
#include "ComponentRegistry.h"


namespace framework {


//---------------
// ComponentPool
//
// Contains a dynamic list of components of a certain type, with the actual type being erased
//
class ComponentPool
{
	// construct destruct
	//--------------------
public:
	ComponentPool(T_CompTypeIdx const typeIdx) : m_ComponentType(typeIdx) {}

	// accessors
	//-----------
	template <typename TComponentType>
	TComponentType& Get(size_t const idx);

	// Data
	///////

private:
	T_CompTypeIdx m_ComponentType;

	void* m_Data = nullptr;

	size_t m_Size = 0u;		// in bytes
	size_t m_Capacity = 0u; // in bytes
};


} // namespace framework


#include "ComponentPool.inl"
