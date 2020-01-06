#include "stdafx.h"
#include "Archetype.h"


namespace fw {


//===========
// Archetype
//===========


//-------------------
// Archetype::c-tor
//
// Generate component pools and an index list for the provided signature
//
Archetype::Archetype(ComponentSignature const& sig) 
	: m_Signature(sig)
{
	// fill the mapping vector so there is a value for each type index in the signature
	T_CompTypeIdx const max = m_Signature.GetMaxComponentType();
	if (max != INVALID_COMP_TYPE_IDX)
	{
		m_Mapping = std::vector<T_CompTypeIdx>(max + 1u, INVALID_COMP_TYPE_IDX);
	}

	// we will add a pool for each type in the signature
	m_ComponentPools.reserve(m_Signature.GetTypes().size());

	// create pools and direct the mapping vector to those pools
	T_CompTypeIdx mappingIdx = 0u;
	for (T_CompTypeIdx const compType : m_Signature.GetTypes())
	{
		m_ComponentPools.emplace_back(compType);
		m_Mapping[compType] = mappingIdx++;
	}
}

//-------------------------
// Archetype::HasComponent
//
// Check if the archetype can store a specific component type
//
bool Archetype::HasComponent(T_CompTypeIdx const compType) const
{
	if (compType >= m_Mapping.size())
	{
		return false;
	}

	return (m_Mapping[compType] != INVALID_COMP_TYPE_IDX);
}

//-------------------------
// Archetype::GetPool
//
// Access the pool of a given component type
//
ComponentPool& Archetype::GetPool(T_CompTypeIdx const typeIdx)
{
	ET_ASSERT(HasComponent(typeIdx));
	return m_ComponentPools[m_Mapping[typeIdx]];
}

//-------------------------
// Archetype::GetPool
//
ComponentPool const& Archetype::GetPool(T_CompTypeIdx const typeIdx) const
{
	ET_ASSERT(HasComponent(typeIdx));
	return m_ComponentPools[m_Mapping[typeIdx]];
}

//-------------------------
// Archetype::GetEntity
//
// Get the ID of the entity which has it's components stored at the index
//
T_EntityId Archetype::GetEntity(size_t const idx) const
{
	ET_ASSERT(idx < m_Entities.size());
	return m_Entities[idx];
}

//----------------------
// Archetype::AddEntity
//
// Add an entity and all related components to an archetype
//  - returns the index within the archetype which the entity occupies
//
// #todo: we might want to simply assume that the components are sorted by signature to add their data for performance reasons
//
size_t Archetype::AddEntity(T_EntityId const entity, std::vector<RawComponentPtr> const& components)
{
	ET_ASSERT(m_Signature.MatchesComponentsUnsorted(components));

	for (RawComponentPtr const& component : components)
	{
		m_ComponentPools[m_Mapping[component.typeIdx]].Append(component.data);
	}

	size_t const idx = m_Entities.size();
	m_Entities.emplace_back(entity);
	return idx;
}

//-------------------------
// Archetype::RemoveEntity
//
// Remove all the component data for an entity from the archetype - returns the entity id that was placed in the current index
//
T_EntityId Archetype::RemoveEntity(size_t const idx)
{
	ET_ASSERT(idx < m_Entities.size());

	for (ComponentPool& pool : m_ComponentPools)
	{
		pool.Erase(idx);
	}

	if (m_Entities.size() == 1u)
	{
		m_Entities.clear();
	}
	else
	{
		m_Entities[idx] = m_Entities[m_Entities.size() - 1];
		m_Entities.pop_back();
	}

	if (m_Entities.size() == idx)
	{
		return INVALID_ENTITY_ID;
	}
	
	return m_Entities[idx];
}

//-------------------------
// Archetype::Clear
//
void Archetype::Clear()
{
	for (ComponentPool& pool : m_ComponentPools)
	{
		pool.Clear();
	}

	m_Entities.clear();
}


} // namespace fw
