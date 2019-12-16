#pragma once
#include "ComponentPool.h"
#include "EntityFwd.h"


namespace framework {


//---------------
// Archetype
//
// Contains a specific set of components for entities that match the given signature
//
class Archetype final
{
	// construct destruct
	//--------------------
public:
	Archetype(std::vector<T_CompTypeIdx> const& compTypes) : m_ComponentTypes(compTypes) { ET_ASSERT(false, "not implemented"); }

	// accessors
	//-----------
	ComponentPool& GetPool(T_CompTypeIdx const typeIdx) { return m_ComponentPools[m_Mapping[typeIdx]]; }

	// Data
	///////

private:
	std::vector<T_CompTypeIdx> m_Mapping; // slot map like -> access pools by component type index

	std::vector<ComponentPool> m_ComponentPools;
	std::vector<T_CompTypeIdx> m_ComponentTypes; // signature

	std::vector<T_EntityId> m_Entities; // map back into the controllers entity list, can also act as component count
};


} // namespace framework
