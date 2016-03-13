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
		std::cout << "AbstractComponent::GetTransform() > Failed to retrieve the TransformComponent. GameObject is NULL." << std::endl;
		return nullptr;
	}
#endif

	return m_pEntity->GetTransform();
}
