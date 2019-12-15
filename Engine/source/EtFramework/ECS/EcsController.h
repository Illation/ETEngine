#pragma once
#include "ComponentRegistry.h"


namespace framework {


class Archetype
{
public:

};


struct ArchetypeContainer
{
	std::vector<Archetype*> archetypes;
};


struct Entity
{
	Archetype const* archetype = nullptr;
	size_t index = 0u;
};


typedef core::T_SlotId T_EntityId;


class EcsController
{
public:

private:
	core::slot_map<Entity> m_Entities;
	std::vector<ArchetypeContainer> m_HierachyLevels;
};


} // namespace framework
