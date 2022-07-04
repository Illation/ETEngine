#pragma once
#include <rttr/type>

#include <EtFramework/ECS/EntityFwd.h>


namespace et {
REGISTRATION_NS(fw);
}


namespace et {
namespace fw {


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
	REGISTRATION_FRIEND_NS(fw)
public:

	// construct destruct
	//--------------------
	EntityLink() = default;
	EntityLink(EntityLink const& other);
	EntityLink& operator=(EntityLink const& other);
	~EntityLink();

	// get / set for serialization
	//-----------------------------
	void SetDeserializedId(T_EntityId const val);
	void SetId(T_EntityId const val) { m_Id = val; m_IsResolved = true; }
	T_EntityId GetId() const { return m_Id; }
	bool IsResolved() const { return m_IsResolved; }

	// Data
	///////

private:
	T_EntityId m_Id = INVALID_ENTITY_ID;
	bool m_IsResolved = false;
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
	void UnregisterLink(EntityLink* const link);
	void Clear();

	// Data
	///////

private:
	std::unordered_map<T_EntityId, std::vector<EntityLink*>> m_RegisteredLinks;
};


} // namespace fw
} // namespace et
