#pragma once
#include "EntityFwd.h"
#include "RawComponentData.h"


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

	// construct destruct
	//--------------------
public:
	EcsController();

	// functionality
	//---------------
	// #todo: support bulk component add / remove with variadic templates

	// entity managment
	T_EntityId AddEntity(T_EntityId const parent = INVALID_ENTITY_ID);
	T_EntityId AddEntity(std::vector<RawComponentData> const& components, T_EntityId const parent = INVALID_ENTITY_ID);

	void ReparentEntity(T_EntityId const entity, T_EntityId const newParent);

	void RemoveEntity(T_EntityId& entity);

	// components
	void* AddComponent(T_EntityId const entity, RawComponentData const& component);
	template<typename TComponentType>
	TComponentType& AddComponent(T_EntityId const entity);
	template<typename TComponentType>
	TComponentType& AddComponent(T_EntityId const entity, TComponentType const& component);

	void AddComponents(T_EntityId const entity, std::vector<RawComponentData> const& components);

	void RemoveComponent(T_EntityId const entity, T_CompTypeIdx const typeIdx);
	template<typename TComponentType>
	void RemoveComponent(T_EntityId const entity);

	void RemoveComponents(T_EntityId const entity, std::vector<T_CompTypeIdx> const& components);

	// systems

	// accessors
	//-----------

	// hierachy
	bool HasParent(T_EntityId const entity) const;

	T_EntityId GetParent(T_EntityId const entity) const;

	std::vector<T_EntityId> const& GetChildren(T_EntityId const entity) const;

	// components
	std::vector<T_CompTypeIdx> const& GetComponentTypes(T_EntityId const entity) const;

	bool HasComponent(T_EntityId const entity, T_CompTypeIdx const compType) const;
	template<typename TComponentType>
	bool HasComponent(T_EntityId const entity);

	template<typename TComponentType>
	TComponentType& GetComponent(T_EntityId const entity);
	template<typename TComponentType>
	TComponentType const& GetComponent(T_EntityId const entity) const;

	void const* GetComponentData(T_EntityId const entity, T_CompTypeIdx const compType) const;

	// Data
	///////

private:
	core::slot_map<EntityData> m_Entities;
	std::vector<ArchetypeContainer> m_HierachyLevels;
};


} // namespace framework
