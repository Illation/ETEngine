#include "stdafx.h"
#include "Entity.h"

#include "AbstractScene.h"

#include <EtFramework/Components/AbstractComponent.h>
#include <EtFramework/Components/TransformComponent.h>


//========
// Entity
//========


// construct destruct
//////////////////////

//---------------------
// Entity::c-tor
//
// Ensure we have a transform component
//
Entity::Entity()
	: m_Transform(new TransformComponent())
{
	AddComponent(m_Transform);
}

//---------------------
// Entity::d-tor
//
Entity::~Entity()
{
	if (m_IsInitialized)
	{
		RootDeinit();
	}

	//Object Cleanup
	for (Entity* child : m_Children)
	{
		SafeDelete(child);
	}

	//Component Cleanup
	for (AbstractComponent* component : m_Components)
	{
		SafeDelete(component);
	}
}


// root
//////////

//---------------------
// Entity::RootInit
//
// Init derived classes, child entities and all components
//
void Entity::RootInit()
{
	if (m_IsInitialized)
	{
		return;
	}

	Init();

	//Root-Component Initialization
	for (AbstractComponent* const component : m_Components)
	{
		component->RootInit();
	}

	//Root-Object Initialization
	for (Entity* const child : m_Children)
	{
		child->RootInit();
	}

	OnPostComponentInit();

	m_IsInitialized = true;
}

//---------------------
// Entity::RootDeinit
//
void Entity::RootDeinit()
{
	if (!m_IsInitialized)
	{
		return;
	}

	Deinit();

	// Root-Object deinit
	for (Entity* const child : m_Children)
	{
		child->RootDeinit();
	}

	// Root-Component deinit
	// iterate in reverse so that transform is removed last
	for (auto it = m_Components.rbegin(); it != m_Components.rend(); it++) 
	{
		(*it)->RootDeinit();
	}

	m_IsInitialized = false;
}

//---------------------
// Entity::RootUpdate
//
// Recursively update all children and components
//
void Entity::RootUpdate()
{
	Update();

	//Component Update
	for (AbstractComponent* const component : m_Components)
	{
		component->Update();
	}

	//Root-Object Update
	for (Entity* const child : m_Children)
	{
		child->RootUpdate();
	}
}


// functionality
/////////////////

//---------------------
// Entity::AddChild
//
void Entity::AddChild(Entity* const entity)
{
	ET_ASSERT(entity->GetParent() == nullptr, "Entity already attatched to another parent!");
	ET_ASSERT(entity->GetScene() == nullptr, "Entity already attatched to another scene!");

	entity->m_ParentEntity = this;
	m_Children.push_back(entity);

	if (m_IsInitialized)
	{
		entity->RootInit();
	}
}

//---------------------
// Entity::RemoveChild
//
void Entity::RemoveChild(Entity* const entity)
{
	auto it = std::find(m_Children.begin(), m_Children.end(), entity);

	ET_ASSERT(it != m_Children.cend(), "Entity to remove is not a child of entity to remove from!");

	m_Children.erase(it); // swap-pop in the future?
	entity->m_ParentEntity = nullptr;
}

//----------------------
// Entity::AddComponent
//
void Entity::AddComponent(AbstractComponent* const component)
{
	ET_ASSERT(!(typeid(*component) == typeid(TransformComponent) && HasComponent<TransformComponent>()), "Entity already has a transform component");
	// #todo: check for components of any type

	component->m_Entity = this;

	if (m_IsInitialized)
	{
		component->RootInit();
	}

	m_Components.push_back(component);
}

//-------------------------
// Entity::RemoveComponent
//
void Entity::RemoveComponent(AbstractComponent* const component)
{
	ET_ASSERT(typeid(*component) == typeid(TransformComponent), "TransformComponent can't be removed!");

	auto it = std::find(m_Components.begin(), m_Components.end(), component);

	ET_ASSERT(it != m_Components.cend(), "Component is not attached to this entity!");

	m_Components.erase(it);
	component->m_Entity = nullptr;
}

//---------------------
// Entity::SetName
//
void Entity::SetName(std::string const& name)
{
	m_Name = name;
	m_Id = GetHash(name);
}


// accessors
/////////////

//---------------------------------
// Entity::RecursiveAppendChildren
//
// Add all children to a flat list
//
void Entity::RecursiveAppendChildren(std::vector<Entity const*>& list) const
{
	list.push_back(this);
	for (Entity const* const child : m_Children)
	{
		child->RecursiveAppendChildren(list);
	}
}

//---------------------------------
// Entity::RecursiveAppendChildren
//
// Add all children to a flat list
//
void Entity::RecursiveAppendChildren(std::vector<Entity*>& list) 
{
	list.push_back(this);
	for (Entity* const child : m_Children)
	{
		child->RecursiveAppendChildren(list);
	}
}

//---------------------
// Entity::GetScene
//
AbstractScene* Entity::GetScene()
{
	if (!m_ParentScene && m_ParentEntity)
	{
		return m_ParentEntity->GetScene();
	}

	return m_ParentScene;
}