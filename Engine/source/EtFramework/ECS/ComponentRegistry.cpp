#include "stdafx.h"
#include "ComponentRegistry.h"


namespace fw {


//====================
// Component Registry
//====================


//-----------------------------
// ComponentRegistry::Instance
//
// Global singleton access
//
ComponentRegistry& ComponentRegistry::Instance()
{
	static ComponentRegistry instance; 
	return instance;
}

//----------------------------
// ComponentRegistry::GetSize
//
// Size in bytes required to store a component
//
size_t ComponentRegistry::GetSize(T_CompTypeIdx const idx) const
{
	ET_ASSERT(static_cast<size_t>(idx) < m_ComponentTypes.size());

	return m_ComponentTypes[idx].data_size;
}

//----------------------------
// ComponentRegistry::GetType
//
// Any other type info related to a component
//
rttr::type const& ComponentRegistry::GetType(T_CompTypeIdx const idx) const
{
	ET_ASSERT(static_cast<size_t>(idx) < m_ComponentTypes.size());

	return m_ComponentTypes[idx].type;
}

//---------------------------------------
// ComponentRegistry::GetCopyConstructor
//
// Function that copies a block of memory with the components copy constructor
//
ComponentRegistry::T_CompCopyAssign ComponentRegistry::GetCopyAssign(T_CompTypeIdx const idx) const
{
	ET_ASSERT(static_cast<size_t>(idx) < m_ComponentTypes.size());

	return m_ComponentTypes[idx].copyAssign;
}

//---------------------------------------
// ComponentRegistry::GetDestructor
//
// Function that calls the components destructor on a block of memory without freeing it
//
ComponentRegistry::T_CompDestructor ComponentRegistry::GetDestructor(T_CompTypeIdx const idx) const
{
	ET_ASSERT(static_cast<size_t>(idx) < m_ComponentTypes.size());

	return m_ComponentTypes[idx].destructor;
}

//---------------------------------------
// ComponentRegistry::GetDestructor
//
// Function that deletes the component
//
ComponentRegistry::T_CompDestructor ComponentRegistry::GetFullDestructor(T_CompTypeIdx const idx) const
{
	ET_ASSERT(static_cast<size_t>(idx) < m_ComponentTypes.size());

	return m_ComponentTypes[idx].fullDestructor;
}

//-------------------------------
// ComponentRegistry::GetTypeIdx
//
T_CompTypeIdx ComponentRegistry::GetTypeIdx(rttr::type const& type) const
{
	auto const foundTiIt = std::find_if(m_ComponentTypes.cbegin(), m_ComponentTypes.cend(), [type](ComponentTypeInfo const& info)
		{
			return info.type == type;
		});

	if (foundTiIt == m_ComponentTypes.cend())
	{
		return s_InvalidTypeIdx;
	}

	return static_cast<T_CompTypeIdx>(foundTiIt - m_ComponentTypes.cbegin());
}


} // namespace fw

