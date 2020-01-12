#pragma once
#include "ComponentRegistry.h"
#include "RawComponentPointer.h"


namespace fw {


class EcsController;


//------------------
// EcsCommandBuffer
//
// Queues modifications to the ECS in a concurrency friendly way, so that systems can modify the entity layout
//  - components added to the buffer cannot be used until the buffer is merged with the ECS
//
//  - Merge order:
//		- Create new empty entities immediately, including duplications, but queue duplicate components for later addition
//
//		- Reparent entities
//		- Remove Components
//		- Add Components - callbacks for entities with events
//		- Remove entities
//
class EcsCommandBuffer final
{
	// definitions
	//-------------
public:
	typedef std::function<void(EcsController&, T_EntityId const)> T_OnMergeFn;

private:
	struct AddBuffer
	{
		std::vector<RawComponentPtr> components;
		std::vector<T_OnMergeFn> callbacks;
	};

	friend class SystemBase;

	// construct destruct
	//--------------------
	EcsCommandBuffer() = default;
	~EcsCommandBuffer();

	// no copying the buffer
	EcsCommandBuffer(EcsCommandBuffer const&) = delete;
	void operator=(EcsCommandBuffer const&) = delete;

	// create new entities
	//---------------------
public:
	T_EntityId AddEntity();
	T_EntityId AddEntityChild(T_EntityId const parent);
	T_EntityId DuplicateEntity(T_EntityId const dupe);

	// modify existing entities
	//--------------------------
	void ReparentEntity(T_EntityId const entity, T_EntityId const newParent);
	void RemoveEntity(T_EntityId const entity);

	// modify component content
	//--------------------------
	template<typename TComponentType, typename... Args>
	void AddComponents(T_EntityId const entity, TComponentType& component1, Args... args);
	void AddComponentList(T_EntityId const entity, std::vector<RawComponentPtr> const& components);

	template<typename TComponentType, typename... Args>
	void RemoveComponents(T_EntityId const entity);
	void RemoveComponentTypes(T_EntityId const entity, T_CompTypeList const& componentTypes);

	// callbacks when adding components to entities
	//----------------------------------------------
	void OnMerge(T_EntityId const entity, T_OnMergeFn& fn);

	// private system control
	//------------------------
private:
	void SetController(EcsController* const ecs) { m_Controller = ecs; }
	void Merge();

	// Data
	///////

	EcsController* m_Controller = nullptr;

	std::vector<std::pair<T_EntityId, T_EntityId>> m_ReparentEntities; // [to reparent, new parent]
	std::vector<T_EntityId> m_RemoveEntities;

	std::unordered_map<T_EntityId, AddBuffer> m_AddComponents;
	std::unordered_map<T_EntityId, T_CompTypeList> m_RemoveComponents;
};


} // namespace fw


#include "EcsCommandBuffer.inl"

