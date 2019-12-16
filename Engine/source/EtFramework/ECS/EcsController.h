#pragma once
#include "EntityFwd.h"


namespace framework {


// fwd
class Archetype;


//---------------
// EcsController
//
// Full context for an entity component system
//
class EcsController final
{
	// definitions
	//-------------
	struct EntityData final
	{
		T_EntityId parent = INVALID_ENTITY_ID;
		std::vector<T_EntityId> children;
		size_t layer = 0u;

		Archetype const* archetype = nullptr;
		size_t index = 0u;
	};

	struct ArchetypeContainer final
	{
		std::vector<Archetype*> archetypes;
	};

public:

private:
	core::slot_map<EntityData> m_Entities;
	std::vector<ArchetypeContainer> m_HierachyLevels;
};


} // namespace framework
