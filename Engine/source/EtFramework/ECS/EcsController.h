#pragma once
#include "ComponentSignature.h"
#include "EcsEvents.h"
#include "EntityFwd.h"
#include "RawComponentPointer.h"
#include "System.h"
#include "Archetype.h"


namespace et {
namespace fw {


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
		uint8 layer = 0u; // deeply nested hierachies are discouraged for performance reasons. 256 layers should be enough

		Archetype* archetype = nullptr;
		size_t index = 0u;
	};

	struct ArchetypeContainer final
	{
		std::unordered_map<T_Hash, Archetype*> archetypes;
	};

	struct RegisteredSystem final
	{
		struct ArchetypeLayer
		{
			std::vector<Archetype*> archetypes;
		};

		RegisteredSystem(SystemBase* const sys) : system(sys), signature(sys->GetSignature()) {} 

		// system
		SystemBase* system;
		ComponentSignature signature;

		// for topological sort
		std::vector<RegisteredSystem*> dependencies;
		bool visited = false;
		bool scheduled = false;

		// component combinations to iterate
		std::vector<ArchetypeLayer> matchingArchetypes;
	};

	// construct destruct
	//--------------------
public:
	EcsController();
	~EcsController();

	// functionality
	//---------------

	// entity managment
	T_EntityId AddEntity();
	T_EntityId AddEntityChild(T_EntityId const parent);
	T_EntityId AddEntityBatched(std::vector<RawComponentPtr> const& components);
	T_EntityId AddEntityBatched(T_EntityId const parent, std::vector<RawComponentPtr> const& components);

	template<typename TComponentType, typename... Args>
	T_EntityId AddEntity(TComponentType& component1, Args... args);
	template<typename TComponentType, typename... Args>
	T_EntityId AddEntityChild(T_EntityId const parent, TComponentType& component1, Args... args);

	T_EntityId DuplicateEntityAddComponents(T_EntityId const dupe, std::vector<RawComponentPtr> const& components);

	template<typename TComponentType, typename... Args>
	T_EntityId DuplicateEntity(T_EntityId const dupe, TComponentType& component1, Args... args);

	void ReparentEntity(T_EntityId const entity, T_EntityId const newParent);

	void RemoveEntity(T_EntityId const entity);
	void RemoveAllEntities();

	// components
	template<typename TComponentType, typename... Args>
	void AddComponents(T_EntityId const entity, TComponentType& component1, Args... args);
	void AddComponents(T_EntityId const entity, std::vector<RawComponentPtr>& components);

	template<typename TComponentType, typename... Args>
	void RemoveComponents(T_EntityId const entity);
	void RemoveComponents(T_EntityId const entity, T_CompTypeList const& componentTypes);

	// component events
	template<typename TComponentType>
	T_CompEventId RegisterOnComponentAdded(T_CompEventFn<TComponentType>& fn);
	template<typename TComponentType>
	T_CompEventId RegisterOnComponentRemoved(T_CompEventFn<TComponentType>& fn);
	template<typename TComponentType>
	void UnregisterComponentEvent(T_CompEventId& callbackId);

	// entity events
	T_EntityEventId RegisterOnEntityAdded(T_EntityEventFn& fn);
	T_EntityEventId RegisterOnEntityRemoved(T_EntityEventFn& fn);
	void UnregisterEntityEvent(T_EntityEventId& callbackId);

	// systems
	void Process(); 

	template<typename TSystemType, typename... Args>
	void RegisterSystem(Args... args);
	template<typename TSystemType>
	void UnregisterSystem();

	template<typename TViewType>
	void ProcessViewOneShot(T_OneShotProcess<TViewType> const& processFn);

	// accessors
	//-----------

	// entities
	size_t GetEntityCount() const { return m_Entities.size(); }
	std::vector<T_EntityId> const& GetEntities() const { return m_Entities.ids(); }
	bool HasParent(T_EntityId const entity) const;
	T_EntityId GetParent(T_EntityId const entity) const;
	std::vector<T_EntityId> const& GetChildren(T_EntityId const entity) const;

	// components
	T_CompTypeList const& GetComponentTypes(T_EntityId const entity) const;

	bool HasComponent(T_EntityId const entity, T_CompTypeIdx const compType) const;
	template<typename TComponentType>
	bool HasComponent(T_EntityId const entity) const;

	template<typename TComponentType>
	TComponentType& GetComponent(T_EntityId const entity);
	template<typename TComponentType>
	TComponentType const& GetComponent(T_EntityId const entity) const;

	void* GetComponentData(T_EntityId const entity, T_CompTypeIdx const compType);
	void const* GetComponentData(T_EntityId const entity, T_CompTypeIdx const compType) const;

	// systems
	template<typename TSystemType>
	bool IsSystemRegistered() const;

	// utility
	//---------
private:
	Archetype* FindOrCreateArchetype(ComponentSignature const& sig, uint8 const layer);
	void MoveArchetype(T_EntityId const entId, EntityData& ent, T_CompTypeList const& compTypes, std::vector<RawComponentPtr> const& components);
	void RemoveEntityFromArchetype(EntityData& ent);
	T_CompTypeList GetComponentsAndTypes(EntityData& ent, std::vector<RawComponentPtr>& components);

	void RemoveEntityFromParent(T_EntityId const entity, T_EntityId const parent);

	void RegisterSystemInternal(SystemBase* const sys);
	void UnregisterSystemInternal(T_SystemType const sysType);

	void RecalculateSystemSchedule();
	void TopologicalSort(RegisteredSystem* const sys);

	// Data
	///////

	core::slot_map<EntityData> m_Entities;

	std::vector<ArchetypeContainer> m_HierachyLevels;

	std::vector<detail::T_ComponentEventDispatcher> m_ComponentEvents;
	detail::T_EntityEventDispatcher m_EntityEvents;

	std::vector<RegisteredSystem*> m_Systems; // system ownership
	std::vector<RegisteredSystem*> m_Schedule; // for iteration
};


} // namespace fw
} // namespace et


#include "EcsController.inl"
