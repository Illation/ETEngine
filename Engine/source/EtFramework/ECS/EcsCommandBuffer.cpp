#include "stdafx.h"
#include "EcsCommandBuffer.h"

#include "EcsController.h"


namespace fw {


//====================
// ECS Command Buffer
//====================


// construct destruct
//////////////////////

//-------------------------
// EcsCommandBuffer::d-tor
//
EcsCommandBuffer::~EcsCommandBuffer()
{
	ET_ASSERT(m_ReparentEntities.empty(), "deleting command buffer before it was merged!");
	ET_ASSERT(m_RemoveEntities.empty(), "deleting command buffer before it was merged!");
	ET_ASSERT(m_AddComponents.empty(), "deleting command buffer before it was merged!");
	ET_ASSERT(m_RemoveComponents.empty(), "deleting command buffer before it was merged!");
	
	for (std::pair<T_EntityId const, std::vector<RawComponentPtr>>& addPair : m_AddComponents)
	{
		for (RawComponentPtr const& ptr : addPair.second)
		{
			// match malloc with free - hence we don't use the full destructor
			ComponentRegistry::Instance().GetDestructor(ptr.typeIdx)(ptr.data);
			free(ptr.data);
		}
	}
}


// create new entities
///////////////////////
//  - this resolves immediately in order to get a correct entity ID, however it is impossible to directly add components in a batched way

//-----------------------------
// EcsCommandBuffer::AddEntity
//
T_EntityId EcsCommandBuffer::AddEntity()
{
	return m_Controller->AddEntity();
}

//----------------------------------
// EcsCommandBuffer::AddEntityChild
//
T_EntityId EcsCommandBuffer::AddEntityChild(T_EntityId const parent)
{
	return m_Controller->AddEntityChild(parent);
}

//-----------------------------------
// EcsCommandBuffer::DuplicateEntity
//
T_EntityId EcsCommandBuffer::DuplicateEntity(T_EntityId const dupe)
{
	// create a new entity with the same parent
	T_EntityId const ret = m_Controller->AddEntityChild(m_Controller->GetParent(dupe));

	// get the entities component list
	T_CompTypeList const& compTypes = m_Controller->GetComponentTypes(dupe);
	std::vector<RawComponentPtr> components;
	for (T_CompTypeIdx const typeId : compTypes)
	{
		components.emplace_back(typeId, m_Controller->GetComponentData(dupe, typeId));
	}

	// copy those components and queue them for later merge into the newly created entity
	AddComponentList(ret, components);

	// return the new entities ID
	return ret;
}


// modify existing entities
////////////////////////////

//----------------------------------
// EcsCommandBuffer::ReparentEntity
//
void EcsCommandBuffer::ReparentEntity(T_EntityId const entity, T_EntityId const newParent)
{
	ET_ASSERT(std::find_if(m_ReparentEntities.cbegin(), m_ReparentEntities.cend(), [entity](std::pair<T_EntityId, T_EntityId> const& pair)
		{
			return (entity == pair.first);
		}) == m_ReparentEntities.cend(), "Entity was already queued for reparenting!");

	m_ReparentEntities.emplace_back(entity, newParent);
}

//----------------------------------
// EcsCommandBuffer::RemoveEntity
//
void EcsCommandBuffer::RemoveEntity(T_EntityId const entity)
{
	ET_ASSERT(std::find(m_RemoveEntities.cbegin(), m_RemoveEntities.cend(), entity) == m_RemoveEntities.cend(), "It's like beating a dead horse!");

	m_RemoveEntities.emplace_back(entity);
}


// modify component content
////////////////////////////

//------------------------------------
// EcsCommandBuffer::AddComponentList
//
void EcsCommandBuffer::AddComponentList(T_EntityId const entity, std::vector<RawComponentPtr> const& components)
{
	// get the component pointer buffer for the entity
	auto buffer = m_AddComponents.emplace(entity, std::vector<RawComponentPtr>()).first;
	
	// add components to buffer
	for (RawComponentPtr const& comp : components)
	{
		// copy construct the component pointer
		RawComponentPtr copy(comp.typeIdx, malloc(ComponentRegistry::Instance().GetSize(comp.typeIdx)));
		ComponentRegistry::Instance().GetCopyAssign(comp.typeIdx)(comp.data, copy.data);

		buffer->second.push_back(copy);
	}
}

//----------------------------------------
// EcsCommandBuffer::RemoveComponentTypes
//
void EcsCommandBuffer::RemoveComponentTypes(T_EntityId const entity, T_CompTypeList const& componentTypes)
{
	// get the component type buffer for the entity
	auto buffer = m_RemoveComponents.emplace(entity, T_CompTypeList()).first;

	// add components to remove to the buffer
	for (T_CompTypeIdx const compType : componentTypes)
	{
		buffer->second.push_back(compType);
	}
}


// private system control
//////////////////////////

//--------------------------
// EcsCommandBuffer::Merge
//
// This function should be called by the system once it finishes executing in order to actually execute the commands
//
void EcsCommandBuffer::Merge()
{
	ET_ASSERT(m_Controller != nullptr);

	// reparent entities
	for (std::pair<T_EntityId, T_EntityId> const repPair : m_ReparentEntities)
	{
		m_Controller->ReparentEntity(repPair.first, repPair.second);
	}

	m_ReparentEntities.clear();

	// remove components
	for (std::pair<T_EntityId, T_CompTypeList> const& remComps : m_RemoveComponents)
	{
		m_Controller->RemoveComponents(remComps.first, remComps.second);
	}

	m_RemoveComponents.clear();

	// add components
	for (std::pair<T_EntityId const, std::vector<RawComponentPtr>>& addComps : m_AddComponents)
	{
		m_Controller->AddComponents(addComps.first, std::vector<RawComponentPtr>(addComps.second));

		// delete the components in the buffer as they have now been copied into the ECS
		for (RawComponentPtr const& ptr : addComps.second)
		{
			// match malloc with free - hence we don't use the full destructor
			ComponentRegistry::Instance().GetDestructor(ptr.typeIdx)(ptr.data);
			free(ptr.data);
		}
	}

	m_AddComponents.clear();

	// remove entities
	for (T_EntityId const remEnt : m_RemoveEntities)
	{
		m_Controller->RemoveEntity(remEnt);
	}

	m_RemoveEntities.clear();
}


} // namespace fw
