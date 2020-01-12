#include "stdafx.h"
#include "EcsController.h"

#include "ComponentSignature.h"
#include "Archetype.h"


namespace fw {


//================
// ECS Controller
//================


// construct destruct
//////////////////////

//--------------------------
// EcsController::c-tor
//	
EcsController::EcsController()
	: m_ComponentEvents(ComponentRegistry::Instance().GetCount())
{ }

//--------------------------
// EcsController::d-tor
//
EcsController::~EcsController()
{
	// delete archetypes
	for (ArchetypeContainer& container : m_HierachyLevels)
	{
		for (std::pair<T_Hash const, Archetype*>& arch : container.archetypes)
		{
			delete arch.second;
		}
	}

	// delete systems
	for (RegisteredSystem* const sys : m_Systems)
	{
		delete sys->system;
		delete sys;
	}
}

// entity managment
/////////////////////

//--------------------------
// EcsController::AddEntity
//
// Add an entity without components to the system
//
T_EntityId EcsController::AddEntity()
{
	return AddEntityChild(INVALID_ENTITY_ID);
}

//-------------------------------
// EcsController::AddEntityChild
//
// Add an entity without components to the system, one level below the parent entity
//
T_EntityId EcsController::AddEntityChild(T_EntityId const parent)
{
	return AddEntityBatched(parent, {});
}

//---------------------------------
// EcsController::AddEntityBatched
//
// Add an entity with a set of components
//
T_EntityId EcsController::AddEntityBatched(std::vector<RawComponentPtr> const& components)
{
	return AddEntityBatched(INVALID_ENTITY_ID, components);
}

//----------------------------------
// EcsController::AddEntityBatched
//
// Add an entity with a set of components
//  - the entity is linked to the parent and placed in the layer below
//
T_EntityId EcsController::AddEntityBatched(T_EntityId const parent, std::vector<RawComponentPtr> const& components)
{
	// create the entity data
	auto ent = m_Entities.insert(EntityData());

	// deal with hierachy issues
	if (parent != INVALID_ENTITY_ID)
	{
		ent.first->parent = parent;

		EntityData& parentEnt = m_Entities[parent];
		parentEnt.children.push_back(ent.second);

		ent.first->layer = parentEnt.layer + 1u;
	}

	// find the archetype for the new component list
	ent.first->archetype = FindOrCreateArchetype(ComponentSignature(components), ent.first->layer);
	ent.first->index = ent.first->archetype->AddEntity(ent.second, components);

	// emit events for the added components
	std::vector<RawComponentPtr> addedComponents;
	GetComponentsAndTypes(*(ent.first), addedComponents);

	for (RawComponentPtr& comp : addedComponents)
	{
		m_ComponentEvents[comp.typeIdx].Notify(detail::E_EcsEvent::Added, new detail::ComponentEventData(this, comp.data, ent.second));
	}

	// entity events
	m_EntityEvents.Notify(detail::E_EcsEvent::Added, new detail::EntityEventData(this, ent.second));

	// return the ID
	return ent.second;
}

//---------------------------------------------
// EcsController::DuplicateEntityAddComponents
//
// Duplicate the existing entity and add additional components
//
T_EntityId EcsController::DuplicateEntityAddComponents(T_EntityId const dupe, std::vector<RawComponentPtr> const& components)
{
	// get referred entities current components
	std::vector<RawComponentPtr> currentComponents;
	T_CompTypeList compTypes = GetComponentsAndTypes(m_Entities[dupe], currentComponents);

	// add the new components
	for (RawComponentPtr const& comp : components)
	{
		compTypes.emplace_back(comp.typeIdx);
		currentComponents.emplace_back(comp);
	}

	auto ent = m_Entities.insert(EntityData());

	EntityData const& dupeEnt = m_Entities[dupe]; 

	// deal with hierachy issues
	if (dupeEnt.parent != INVALID_ENTITY_ID)
	{
		ent.first->parent = dupeEnt.parent;

		EntityData& parentEnt = m_Entities[dupeEnt.parent];
		parentEnt.children.push_back(ent.second);

		ent.first->layer = parentEnt.layer + 1u;
	}

	// find the archetype for the new component list
	ent.first->archetype = FindOrCreateArchetype(ComponentSignature(currentComponents), ent.first->layer);
	ent.first->index = ent.first->archetype->AddEntity(ent.second, currentComponents);

	// emit events for the added components
	std::vector<RawComponentPtr> addedComponents;
	GetComponentsAndTypes(*(ent.first), addedComponents);

	for (RawComponentPtr& comp : addedComponents)
	{
		m_ComponentEvents[comp.typeIdx].Notify(detail::E_EcsEvent::Added, new detail::ComponentEventData(this, comp.data, ent.second));
	}

	// entity events
	m_EntityEvents.Notify(detail::E_EcsEvent::Added, new detail::EntityEventData(this, ent.second));

	// return the ID
	return ent.second;
}

//-------------------------------
// EcsController::ReparentEntity
//
// Change what parent an entity is linked to, also moving all of its children
//  - if the parent is not set to INVALID_ENTITY_ID, the entity is linked to the parent and placed in the layer below
//
void EcsController::ReparentEntity(T_EntityId const entity, T_EntityId const newParent)
{
	// get referred entity
	EntityData& ent = m_Entities[entity];

	// remove from current parent
	RemoveEntityFromParent(entity, ent.parent);

	std::vector<RawComponentPtr> components;
	T_CompTypeList compTypes = GetComponentsAndTypes(ent, components);

	// add to new parent, and get new layer
	ent.parent = newParent;
	if (ent.parent != INVALID_ENTITY_ID)
	{
		EntityData& parentEnt = m_Entities[ent.parent];
		parentEnt.children.push_back(entity);

		ent.layer = parentEnt.layer + 1u;
	}
	else
	{
		ent.layer = 0u;
	}

	// #todo: handle cases in which the hierachy level didn't change
	// the archetype will have the same signature but reside on a new hierachy layer
	MoveArchetype(entity, ent, compTypes, components);

	// recursively reparent children to match hierachy layers
	for (T_EntityId const childId : ent.children)
	{
		ReparentEntity(childId, entity);
	}
}

//-----------------------------
// EcsController::RemoveEntity
//
// Remove an entity and all its children
//
void EcsController::RemoveEntity(T_EntityId const entity)
{
	// entity events
	m_EntityEvents.Notify(detail::E_EcsEvent::Removed, new detail::EntityEventData(this, entity));

	// get referred entity
	EntityData& ent = m_Entities[entity];

	// emit events for the entities components
	std::vector<RawComponentPtr> components;
	GetComponentsAndTypes(ent, components);
	for (RawComponentPtr& comp : components)
	{
		m_ComponentEvents[comp.typeIdx].Notify(detail::E_EcsEvent::Removed, new detail::ComponentEventData(this, comp.data, entity));
	}

	RemoveEntityFromParent(entity, ent.parent);

	RemoveEntityFromArchetype(ent);

	// recursively remove children
	for (T_EntityId const childId : ent.children)
	{
		RemoveEntity(childId);
	}

	// remove from entity map
	m_Entities.erase(entity);
}

//----------------------------------
// EcsController::RemoveAllEntities
//
// While the entities and their components are removed, the now empty archetypes persist
//
void EcsController::RemoveAllEntities()
{
	// emit remove events for entities
	if (m_EntityEvents.GetListenerCount() > 0u) // ensure its worth iterating
	{
		std::vector<T_EntityId> const& entities = GetEntities();
		for (T_EntityId const entity : entities)
		{
			m_EntityEvents.Notify(detail::E_EcsEvent::Removed, new detail::EntityEventData(this, entity));
		}
	}

	// emit remove events for components
	for (ArchetypeContainer& level : m_HierachyLevels)
	{
		for (std::pair<T_Hash const, Archetype*>& arch : level.archetypes)
		{
			size_t const entityCount = arch.second->GetSize();
			if (entityCount > 0u) // ensure its worth iterating
			{
				for (ComponentPool& pool : arch.second->GetPools())
				{
					detail::T_ComponentEventDispatcher& events = m_ComponentEvents[pool.GetType()];

					if (events.GetListenerCount() > 0u) // ensure its worth iterating
					{
						for (size_t idx = 0u; idx < entityCount; ++idx)
						{
							events.Notify(detail::E_EcsEvent::Removed, new detail::ComponentEventData(this, pool.At(idx), arch.second->GetEntity(idx)));
						}
					}
				}
			}
		}
	}

	// remove entites
	m_Entities.clear();

	// remove components
	for (ArchetypeContainer& level : m_HierachyLevels)
	{
		for (std::pair<T_Hash const, Archetype*>& arch : level.archetypes)
		{
			arch.second->Clear();
		}
	}
}


// component managment
///////////////////////

//-----------------------------
// EcsController::AddComponent
//
// Add a list of components to an entity
//
void EcsController::AddComponents(T_EntityId const entity, std::vector<RawComponentPtr>& components)
{
	// get referred entity
	EntityData& ent = m_Entities[entity];

	std::vector<RawComponentPtr> currentComponents;
	T_CompTypeList compTypes = GetComponentsAndTypes(ent, currentComponents);

	// add the new components
	for (RawComponentPtr const& comp : components)
	{
		compTypes.emplace_back(comp.typeIdx);
		currentComponents.emplace_back(comp);
	}

	MoveArchetype(entity, ent, compTypes, currentComponents);

	// reassign the component pointers and emit component add events
	for (RawComponentPtr& comp : components)
	{
		comp.data = ent.archetype->GetPool(comp.typeIdx).At(ent.index);
		m_ComponentEvents[comp.typeIdx].Notify(detail::E_EcsEvent::Added, new detail::ComponentEventData(this, comp.data, entity));
	}
}

//---------------------------------
// EcsController::RemoveComponents
//
// Remove a list of components from an entity
//
void EcsController::RemoveComponents(T_EntityId const entity, T_CompTypeList const& componentTypes)
{
	// get referred entity
	EntityData& ent = m_Entities[entity];

	std::vector<RawComponentPtr> currentComponents;
	T_CompTypeList compTypes = GetComponentsAndTypes(ent, currentComponents);

	// remove the components and emit events for them
	for (T_CompTypeIdx const comp : componentTypes)
	{
		if (compTypes.size() == 1u)
		{
			ET_ASSERT(compTypes[0u] == comp);
			compTypes.clear();

			ET_ASSERT(currentComponents.size() == 1u);
			ET_ASSERT(currentComponents[0u].typeIdx == comp);
			m_ComponentEvents[comp].Notify(detail::E_EcsEvent::Removed, new detail::ComponentEventData(this, currentComponents[0u].data, entity));
			currentComponents.clear();
		}
		else
		{
			auto const foundType = std::find(compTypes.begin(), compTypes.end(), comp);
			ET_ASSERT(foundType != compTypes.cend());

			size_t const idx = foundType - compTypes.cbegin();
			ET_ASSERT(compTypes[idx] == currentComponents[idx].typeIdx);

			compTypes[idx] = compTypes[compTypes.size() - 1];
			compTypes.pop_back();

			m_ComponentEvents[comp].Notify(detail::E_EcsEvent::Removed, new detail::ComponentEventData(this, currentComponents[idx].data, entity));
			currentComponents[idx] = currentComponents[currentComponents.size() - 1];
			currentComponents.pop_back();
		}
	}

	MoveArchetype(entity, ent, compTypes, currentComponents);
}


// entity events
/////////////////

//-----------------------------------------
// EcsController::RegisterOnEntityAdded
//
T_EntityEventId EcsController::RegisterOnEntityAdded(T_EntityEventFn& fn)
{
	return m_EntityEvents.Register(detail::E_EcsEvent::Added, detail::T_EntityEventCallbackInternal(
		[this, fn](detail::T_EcsEvent const flags, detail::EntityEventData const* const evnt) -> void
		{
			UNUSED(flags);
			fn(*evnt->controller, evnt->entity);
		}));
}

//-------------------------------------------
// EcsController::RegisterOnEntityRemoved
//
T_EntityEventId EcsController::RegisterOnEntityRemoved(T_EntityEventFn& fn)
{
	return m_EntityEvents.Register(detail::E_EcsEvent::Removed, detail::T_EntityEventCallbackInternal(
		[this, fn](detail::T_EcsEvent const flags, detail::EntityEventData const* const evnt) -> void
		{
			UNUSED(flags);
			fn(*evnt->controller, evnt->entity);
		}));
}

//--------------------------------------
// EcsController::UnregisterEntityEvent
//
// Not required to be called unless the event is a member function that gets deleted before the ecs controller does
//
void EcsController::UnregisterEntityEvent(T_EntityEventId& callbackId)
{
	m_EntityEvents.Unregister(callbackId);
}


// system managment
////////////////////

//------------------------
// EcsController::Process
//
// Update all systems according to their implicit schedule
//
void EcsController::Process()
{
	for (RegisteredSystem* const sys : m_Schedule)
	{
		sys->system->SetCommandController(this);

		for (RegisteredSystem::ArchetypeLayer& layer : sys->matchingArchetypes)
		{
			for (Archetype* const arch : layer.archetypes)
			{
				if (arch->GetSize() > 0u)
				{
					sys->system->RootProcess(this, arch, 0u, arch->GetSize());
				}
			}
		}

		sys->system->MergeCommands();
	}
}


// accessors
/////////////

//--------------------------
// EcsController::HasParent
//
bool EcsController::HasParent(T_EntityId const entity) const
{
	return (GetParent(entity) != INVALID_ENTITY_ID);
}

//--------------------------
// EcsController::GetParent
//
T_EntityId EcsController::GetParent(T_EntityId const entity) const
{
	return m_Entities[entity].parent;
}

//----------------------------
// EcsController::GetChildren
//
std::vector<T_EntityId> const& EcsController::GetChildren(T_EntityId const entity) const
{
	return m_Entities[entity].children;
}

//----------------------------------
// EcsController::GetComponentTypes
//
std::vector<T_CompTypeIdx> const& EcsController::GetComponentTypes(T_EntityId const entity) const
{
	return m_Entities[entity].archetype->GetSignature().GetTypes();
}

//-----------------------------
// EcsController::HasComponent
//
bool EcsController::HasComponent(T_EntityId const entity, T_CompTypeIdx const compType) const
{
	return m_Entities[entity].archetype->HasComponent(compType);
}

//---------------------------------
// EcsController::GetComponentData
//
void* EcsController::GetComponentData(T_EntityId const entity, T_CompTypeIdx const compType) 
{
	EntityData& ent = m_Entities[entity];

	return ent.archetype->GetPool(compType).At(ent.index);
}

//---------------------------------
// EcsController::GetComponentData
//
void const* EcsController::GetComponentData(T_EntityId const entity, T_CompTypeIdx const compType) const
{
	EntityData const& ent = m_Entities[entity];

	return ent.archetype->GetPool(compType).At(ent.index);
}


// utility
///////////

//--------------------------------------
// EcsController::FindOrCreateArchetype
//
// Get the associated archetype or create a new one
//
Archetype* EcsController::FindOrCreateArchetype(ComponentSignature const& sig, uint8 const layer)
{
	T_Hash const sigId = sig.GenId();

	// ensure we have an archetype container for the hierachy layer we are on
	while (m_HierachyLevels.size() <= static_cast<size_t>(layer))
	{
		m_HierachyLevels.emplace_back(ArchetypeContainer());
	}

	// find or create
	ArchetypeContainer& cont = m_HierachyLevels[layer];
	auto foundA = cont.archetypes.find(sigId);
	if (foundA == cont.archetypes.cend())
	{
		auto res = cont.archetypes.emplace(sigId, new Archetype(sig));
		ET_ASSERT(res.second == true);
		foundA = res.first;

		// upon creation, check if there are any systems that should iterate this archetype
		for (RegisteredSystem* const sys : m_Systems)
		{
			if (foundA->second->GetSignature().Contains(sys->signature))
			{
				while (sys->matchingArchetypes.size() <= static_cast<size_t>(layer)) // ensure we have enough layers
				{
					sys->matchingArchetypes.push_back(RegisteredSystem::ArchetypeLayer());
				}

				sys->matchingArchetypes[layer].archetypes.push_back(foundA->second);
			}
		}
	}

	return foundA->second;
}

//------------------------------
// EcsController::MoveArchetype
//
// Move an entities component data to the correct archetype
//
void EcsController::MoveArchetype(T_EntityId const entId, EntityData& ent, T_CompTypeList const& compTypes, std::vector<RawComponentPtr> const& components)
{
	// find the archetype for the new component list
	Archetype* const nextA = FindOrCreateArchetype(ComponentSignature(compTypes), ent.layer);

	// add our entity to the next archetype
	size_t const nextIdx = nextA->AddEntity(entId, components);

	RemoveEntityFromArchetype(ent);

	// reassign the current entity
	ent.archetype = nextA;
	ent.index = nextIdx;
}

//------------------------------------------
// EcsController::RemoveEntityFromArchetype
//
// Remove the entity data from its current archetype, and ensure swapping is correctly handled
//
void EcsController::RemoveEntityFromArchetype(EntityData& ent)
{
	T_EntityId const changedEnt = ent.archetype->RemoveEntity(ent.index);
	if (changedEnt != INVALID_ENTITY_ID) // since the archetype will swap, we need to reassign the index of the entity that was swapped in
	{
		m_Entities[changedEnt].index = ent.index;
	}
}

//--------------------------------------
// EcsController::GetComponentsAndTypes
//
T_CompTypeList EcsController::GetComponentsAndTypes(EntityData& ent, std::vector<RawComponentPtr>& components)
{
	T_CompTypeList compTypes = ent.archetype->GetSignature().GetTypes();
	for (T_CompTypeIdx const type : compTypes)
	{
		components.emplace_back(type, ent.archetype->GetPool(type).At(ent.index));
	}

	return compTypes;
}

//------------------------------
// EcsController::MoveArchetype
//
// Unmap the entity from the parent - assumes parent is actually a parent of the entity
//
void EcsController::RemoveEntityFromParent(T_EntityId const entity, T_EntityId const parent)
{
	if (parent != INVALID_ENTITY_ID)
	{
		EntityData& parentEnt = m_Entities[parent];
		if (parentEnt.children.size() == 1u)
		{
			ET_ASSERT(parentEnt.children[0u] == entity);
			parentEnt.children.clear();
		}
		else
		{
			auto const foundChild = std::find(parentEnt.children.begin(), parentEnt.children.end(), entity);
			ET_ASSERT(foundChild != parentEnt.children.cend());

			size_t const idx = foundChild - parentEnt.children.cbegin();
			parentEnt.children[idx] = parentEnt.children[parentEnt.children.size() - 1];
			parentEnt.children.pop_back();
		}
	}
}

//---------------------------------------
// EcsController::RegisterSystemInternal
//
void EcsController::RegisterSystemInternal(SystemBase* const sys)
{
	// create registered system
	RegisteredSystem* const registered = new RegisteredSystem(sys);

	// add to dependencies
	for (RegisteredSystem* const other : m_Systems)
	{
		if (std::find(sys->GetDependencies().cbegin(), sys->GetDependencies().cend(), other->system->GetTypeId()) != sys->GetDependencies().cend())
		{
			registered->dependencies.emplace_back(other);
		}

		if (std::find(sys->GetDependents().cbegin(), sys->GetDependents().cend(), other->system->GetTypeId()) != sys->GetDependents().cend())
		{
			other->dependencies.emplace_back(registered);
		}
	}

	// add existing archetypes
	for (uint8 layer = 0u; layer < static_cast<uint8>(m_HierachyLevels.size()); ++layer)
	{
		for (std::pair<T_Hash const, Archetype*>& arch : m_HierachyLevels[layer].archetypes)
		{
			// go layer wise to ensure correct hierachy dependency resolution
			if (arch.second->GetSignature().Contains(registered->signature))
			{
				while (registered->matchingArchetypes.size() <= static_cast<size_t>(layer)) // ensure we have enough layers
				{
					registered->matchingArchetypes.push_back(RegisteredSystem::ArchetypeLayer());
				}

				registered->matchingArchetypes[layer].archetypes.push_back(arch.second);
			}
		}
	}

	// add to system list
	m_Systems.emplace_back(registered);

	RecalculateSystemSchedule();
}

//-----------------------------------------
// EcsController::UnregisterSystemInternal
//
void EcsController::UnregisterSystemInternal(T_SystemType const sysType)
{
	// find the registered system
	auto foundSys = std::find_if(m_Systems.cbegin(), m_Systems.cend(), [sysType](RegisteredSystem const* const sys)
		{
			return (sys->system->GetTypeId() == sysType);
		});
	ET_ASSERT(foundSys != m_Systems.cend());

	RegisteredSystem* const registered = *foundSys;

	// remove from system list - swap and remove
	uint8 const idx = static_cast<uint8>(foundSys - m_Systems.cbegin());
	if (idx != m_Systems.size() - 1)
	{
		m_Systems[idx] = m_Systems[m_Systems.size() - 1];
	}

	m_Systems.pop_back();

	// since there are other systems left, remove this system from their dependencies
	for (RegisteredSystem* const other : m_Systems)
	{
		auto const foundDep = std::find(other->dependencies.cbegin(), other->dependencies.cend(), registered);

		if (foundDep != other->dependencies.cend())
		{
			// swap and remove
			size_t const depIdx = foundDep - other->dependencies.cbegin();
			if (depIdx != other->dependencies.size() - 1)
			{
				other->dependencies[depIdx] = other->dependencies[other->dependencies.size() - 1];
			}

			other->dependencies.pop_back();
		}
	}

	// delete the system
	delete registered->system;
	delete registered;

	RecalculateSystemSchedule();
}

//------------------------------------------
// EcsController::RecalculateSystemSchedule
//
// Sort the update schedule by system dependencies
//
void EcsController::RecalculateSystemSchedule()
{
	// reset
	m_Schedule.clear();
	for (RegisteredSystem* const sys : m_Systems)
	{
		sys->visited = false;
		sys->scheduled = false;
	}

	// sort
	for (RegisteredSystem* const sys : m_Systems)
	{
		TopologicalSort(sys);
	}
}

//------------------------------------------
// EcsController::TopologicalSort
//
// Adds systems to the schedule in a valid order according to their dependencies
//
void EcsController::TopologicalSort(RegisteredSystem* const sys)
{
	if (!sys->scheduled)
	{
		ET_ASSERT(!(sys->visited), "Circular dependency detected!");

		sys->visited = true;
		for (RegisteredSystem* const dep : sys->dependencies)
		{
			TopologicalSort(dep);
		}

		sys->scheduled = true;
		m_Schedule.emplace_back(sys);
	}
}


} // namespace fw
