#pragma once
#include <rttr/type>

#include <EtFramework/ECS/EntityFwd.h>


namespace framework {


//------------
// EntityLink
//
// Pointer to an entity that can be rerouted after deserialization
//
struct EntityLink final
{
	// definitions
	//---------------
	RTTR_ENABLE()
public:

	// get / set for serialization
	//-----------------------------
	void SetDeserializedId(T_EntityId const val);
	T_EntityId GetId() const { return id; }

	// Data
	///////

	T_EntityId id = INVALID_ENTITY_ID;
};

//--------------------
// EntityLinkResolver
//
// Pointer to an entity that can be rerouted after deserialization
//
class EntityLinkResolver final
{
	// static access
	//---------------
public:
	static EntityLinkResolver& Instance();

	// construct destruct
	//--------------------
private:
	EntityLinkResolver() = default;
public:
	EntityLinkResolver(EntityLinkResolver const&) = delete;
	void operator=(EntityLinkResolver const&) = delete;

	// functionality
	//---------------
	void OnEntityIdAssigned(T_EntityId const serialized, T_EntityId const assigned);
	void RegisterLink(EntityLink* const link);
	void Clear();

	// Data
	///////

private:
	std::unordered_map<T_EntityId, std::vector<EntityLink*>> m_RegisteredLinks;
};


} // namespace framework
