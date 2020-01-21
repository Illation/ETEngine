#include "stdafx.h"
#include "EntityLink.h"

#include <rttr/registration>


namespace et {
namespace fw {


//=============
// Entity Link 
//=============


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::EntityLink>("entity link")
		.property("id", &fw::EntityLink::GetId, &fw::EntityLink::SetDeserializedId);
}


//-------------------
// EntityLink::c-tor
//
// also register a copy of the link
//
EntityLink::EntityLink(EntityLink const& other)
{
	*this = other;
}

//----------------
// EntityLink:: =
//
// <see above>
//
EntityLink& EntityLink::operator=(EntityLink const& other)
{
	if (id != INVALID_ENTITY_ID)
	{
		EntityLinkResolver::Instance().UnregisterLink(this);
	}

	id = other.id;
	if (id != INVALID_ENTITY_ID)
	{
		EntityLinkResolver::Instance().RegisterLink(this);
	}

	return *this;
}

//-------------------
// EntityLink::d-tor
//
// unregister ourselves upon destruction
//
EntityLink::~EntityLink()
{
	if (id != INVALID_ENTITY_ID)
	{
		EntityLinkResolver::Instance().UnregisterLink(this);
	}
}

//-------------------------------
// EntityLink::SetDeserializedId
//
// Set the id and register with the link resover
//
void EntityLink::SetDeserializedId(T_EntityId const val)
{
	id = val;
	if (id != INVALID_ENTITY_ID)
	{
		EntityLinkResolver::Instance().RegisterLink(this);
	}
}


//====================
// Entity Link Resolver
//====================


//------------------------------
// EntityLinkResolver::Instance
//
// Global singleton access
//
EntityLinkResolver& EntityLinkResolver::Instance()
{
	static EntityLinkResolver instance;
	return instance;
}

//----------------------------------------
// EntityLinkResolver::OnEntityIdAssigned
//
// This function should be called when entities are added to the ECS - all links pointing to the serialized entity will be rerouted
//
void EntityLinkResolver::OnEntityIdAssigned(T_EntityId const serialized, T_EntityId const assigned)
{
	// find all entity links associated with the serialized ID
	auto const foundIt = m_RegisteredLinks.find(serialized);

	// remap all their IDs
	if (foundIt != m_RegisteredLinks.cend())
	{
		for (EntityLink* const link : foundIt->second)
		{
			link->id = assigned;
		}
	}
}

//----------------------------------
// EntityLinkResolver::RegisterLink
//
// This function should be called upon deserializing an entity link
//
void EntityLinkResolver::RegisterLink(EntityLink* const link)
{
	auto const foundIt = m_RegisteredLinks.find(link->id);

	// if we don't have a list of entities for the serialized id yet, create a new one
	if (foundIt == m_RegisteredLinks.cend())
	{
		auto res = m_RegisteredLinks.emplace(link->id, std::vector<EntityLink*>({ link }));
		ET_ASSERT(res.second);
	}
	else // otherwise just add ours
	{
		foundIt->second.push_back(link);
	}
}

//------------------------------------
// EntityLinkResolver::UnregisterLink
//
void EntityLinkResolver::UnregisterLink(EntityLink* const link)
{
	// find the list of links for the id
	auto const foundIt = m_RegisteredLinks.find(link->id);
	if (foundIt == m_RegisteredLinks.cend())
	{
		return;
	}

	// find the actual link
	auto const foundLink = std::find(foundIt->second.begin(), foundIt->second.end(), link);
	if (foundLink == foundIt->second.cend())
	{
		return;
	}

	// remove it
	if (foundIt->second.size() > 1u)
	{
		std::iter_swap(foundLink, std::prev(foundIt->second.end()));
		foundIt->second.pop_back();
	}
	else
	{
		foundIt->second.clear();
	}
}

//---------------------------
// EntityLinkResolver::Clear
//
// After all entities have been deserialized, we can clear the list
//
void EntityLinkResolver::Clear()
{
	m_RegisteredLinks.clear();
}


} // namespace fw
} // namespace et
