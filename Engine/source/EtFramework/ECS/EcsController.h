#pragma once
#include "EntityFwd.h"
#include "RawComponentPointer.h"


namespace framework {


// fwd
class Archetype;
class SystemBase;
class ComponentSignature;


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
			uint8 layer;
			std::vector<Archetype*> archetypes;
		};

		SystemBase* system;
		std::vector<ArchetypeLayer> matchingArchetypes;
		std::vector<RegisteredSystem const*> dependencies;
	};

	// construct destruct
	//--------------------
public:
	EcsController() = default;
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

	// systems
	void Update(); // Process all systems

	template<typename TSystemType>
	void RegisterSystem();
	template<typename TSystemType>
	void UnregisterSystem();

	// accessors
	//-----------

	// entities
	size_t GetEntityCount() const { return m_Entities.size(); }
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

	// Data
	///////

	core::slot_map<EntityData> m_Entities;

	std::vector<ArchetypeContainer> m_HierachyLevels;

	std::vector<RegisteredSystem*> m_Systems;
	std::vector<std::vector<RegisteredSystem*>> m_UpdateGraph;
};


} // namespace framework


#include "EcsController.inl"
