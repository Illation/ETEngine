#include "stdafx.hpp"
#include "Entity.hpp"
#include "AbstractScene.hpp"

#include <iostream>
#include "../Components/AbstractComponent.hpp"
#include "../Components/TransformComponent.hpp"
Entity::Entity():m_Tag(std::string(""))
{
	m_pTransform = new TransformComponent();
	AddComponent(m_pTransform);
}

Entity::~Entity()
{
	//Component Cleanup
	for (AbstractComponent* pComp : m_pComponentVec)
	{
		SafeDelete(pComp);
	}

	//Object Cleanup
	for (Entity* pChild : m_pChildVec)
	{
		SafeDelete(pChild);
	}
}

void Entity::RootInitialize()
{
	if (m_IsInitialized)
		return;
	Initialize();
	//Root-Component Initialization
	for (AbstractComponent* pComp : m_pComponentVec)
	{
		pComp->RootInitialize();
	}
	//Root-Object Initialization
	for (Entity* pChild : m_pChildVec)
	{
		pChild->RootInitialize();
	}
	RootStart();
	m_IsInitialized = true;
}
void Entity::RootStart()
{
	Start();
}
void Entity::RootUpdate()
{
	Update();
	//Component Update
	for (AbstractComponent* pComp : m_pComponentVec)
	{
		pComp->Update();
	}
	//Root-Object Update
	for (Entity* pChild : m_pChildVec)
	{
		pChild->RootUpdate();
	}
}
void Entity::RootDraw()
{
	Draw();
	//Component Draw
	for (AbstractComponent* pComp : m_pComponentVec)
	{
		pComp->Draw();
	}
	//Root-Object Draw
	for (Entity* pChild : m_pChildVec)
	{
		pChild->RootDraw();
	}
}
void Entity::RootDrawForward()
{
	DrawForward();
	//Component Draw
	for (AbstractComponent* pComp : m_pComponentVec)
	{
		pComp->DrawForward();
	}
	//Root-Object Draw
	for (Entity* pChild : m_pChildVec)
	{
		pChild->RootDrawForward();
	}
}
void Entity::RootDrawShadow()
{
	DrawShadow();
	//Component Draw
	for (AbstractComponent* pComp : m_pComponentVec)
	{
		pComp->DrawShadow();
	}
	//Root-Object Draw
	for (Entity* pChild : m_pChildVec)
	{
		pChild->RootDrawShadow();
	}
}

void Entity::AddChild(Entity* pEntity)
{
#if _DEBUG
	if (pEntity->m_pParentEntity)
	{
		if (pEntity->m_pParentEntity == this)
		{
			LOG("Entity::AddChild > Entity to add is already attached to this parent", Warning);
			return;
		}
		LOG("Entity::AddChild > Entity to add is already attached to another GameObject. Detach it from it's current parent before attaching it to another one.", Warning);
		return;
	}

	if (pEntity->m_pParentScene)
	{
		LOG("Entity::AddChild > Entity is currently attached to a Scene. Detach it from it's current parent before attaching it to another one.", Warning);
		return;
	}
#endif

	pEntity->m_pParentEntity = this;
	m_pChildVec.push_back(pEntity);
	if (m_IsInitialized)
	{
		pEntity->RootInitialize();
	}
}

void Entity::RemoveChild(Entity* pEntity)
{
	auto it = find(m_pChildVec.begin(), m_pChildVec.end(), pEntity);

#if _DEBUG
	if (it == m_pChildVec.end())
	{
		LOG("GameObject::RemoveChild > GameObject to remove is not attached to this GameObject!", Warning);
		return;
	}
#endif

	m_pChildVec.erase(it);
	pEntity->m_pParentEntity = nullptr;
}

void Entity::AddComponent(AbstractComponent* pComp)
{
#if _DEBUG
	if (typeid(*pComp) == typeid(TransformComponent) && HasComponent<TransformComponent>())
	{
		LOG("GameObject::AddComponent > GameObject can contain only one TransformComponent!", Warning);
		return;
	}

	for (auto *component : m_pComponentVec)
	{
		if (component == pComp)
		{
			LOG("GameObject::AddComponent > GameObject already contains this component!", Warning);
			return;
		}
	}
#endif

	pComp->m_pEntity = this;
	if (m_IsInitialized)
		pComp->RootInitialize();

	m_pComponentVec.push_back(pComp);
}

void Entity::RemoveComponent(AbstractComponent* pComp)
{
	auto it = find(m_pComponentVec.begin(), m_pComponentVec.end(), pComp);

#if _DEBUG
	if (it == m_pComponentVec.end())
	{
		LOG("GameObject::RemoveComponent > Component is not attached to this GameObject!", Warning);
		return;
	}

	if (typeid(*pComp) == typeid(TransformComponent))
	{
		LOG( "GameObject::RemoveComponent > TransformComponent can't be removed!", Warning);
		return;
	}
#endif

	m_pComponentVec.erase(it);
	pComp->m_pEntity = nullptr;
}

AbstractScene* Entity::GetScene()
{
	if (!m_pParentScene && m_pParentEntity)
	{
		return m_pParentEntity->GetScene();
	}
	return m_pParentScene;
}