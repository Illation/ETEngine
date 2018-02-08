#include "stdafx.hpp"
#include "AbstractComponent.hpp"
#include <iostream>
#include "../SceneGraph/Entity.hpp"

AbstractComponent::AbstractComponent(void)
{
}
AbstractComponent::~AbstractComponent(void)
{
}

void AbstractComponent::RootInitialize()
{
	if (m_IsInitialized)
		return;

	Initialize();

	m_IsInitialized = true;
}

void AbstractComponent::PostDraw()
{
}

TransformComponent* AbstractComponent::GetTransform() const
{
#if _DEBUG
	if (!m_pEntity)
	{
		LOG("AbstractComponent::GetTransform() > Failed to retrieve the TransformComponent. GameObject is NULL.", Warning);
		return nullptr;
	}
#endif

	return m_pEntity->GetTransform();
}
