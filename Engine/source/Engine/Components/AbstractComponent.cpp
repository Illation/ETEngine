#include "stdafx.h"
#include "AbstractComponent.h"

#include <iostream>

#include <Engine/SceneGraph/Entity.h>


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
#if ET_DEBUG
	if (!m_pEntity)
	{
		LOG("AbstractComponent::GetTransform() > Failed to retrieve the TransformComponent. GameObject is NULL.", Warning);
		return nullptr;
	}
#endif

	return m_pEntity->GetTransform();
}
