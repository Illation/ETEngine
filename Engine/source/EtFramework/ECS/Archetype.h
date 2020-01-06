#pragma once
#include "ComponentPool.h"
#include "ComponentSignature.h"
#include "EntityFwd.h"


namespace fw {


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
	Archetype(ComponentSignature const& sig);

	// accessors
	//-----------
	bool HasComponent(T_CompTypeIdx const compType) const;
	ComponentSignature const& GetSignature() const { return m_Signature; }

	size_t GetSize() const { return m_Entities.size(); }

	ComponentPool& GetPool(T_CompTypeIdx const typeIdx);
	ComponentPool const& GetPool(T_CompTypeIdx const typeIdx) const;

	T_EntityId GetEntity(size_t const idx) const;

	// functionality
	//---------------
	size_t AddEntity(T_EntityId const entity, std::vector<RawComponentPtr> const& components);
	T_EntityId RemoveEntity(size_t const idx);
	void Clear();

	// Data
	///////

private:
	std::vector<T_CompTypeIdx> m_Mapping; // slot_map alike -> access pools by component type index

	std::vector<ComponentPool> m_ComponentPools;
	ComponentSignature const m_Signature; 

	std::vector<T_EntityId> m_Entities; // map back into the controllers entity list, can also act as component count
};


} // namespace fw
