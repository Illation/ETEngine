#include "stdafx.h"
#include "AbstractComponent.h"

#include <EtFramework/SceneGraph/Entity.h>


//====================
// Abstract Component
//====================


//-----------------------------
// AbstractComponent::d-tor
//
AbstractComponent::~AbstractComponent()
{
	if (m_IsInitialized)
	{
		RootDeinit();
	}
}

//-----------------------------
// AbstractComponent::RootInit
//
void AbstractComponent::RootInit()
{
	if (m_IsInitialized)
	{
		return;
	}

	Init();

	m_IsInitialized = true;
}

//-------------------------------
// AbstractComponent::RootDeinit
//
void AbstractComponent::RootDeinit()
{
	if (!m_IsInitialized)
	{
		return;
	}

	Deinit();

	m_IsInitialized = false;
}

//---------------------------------
// AbstractComponent::GetTransform
//
TransformComponent* AbstractComponent::GetTransform() const
{
	ET_ASSERT(m_Entity != nullptr, "Can't access transform as not attached to entity!");

	return m_Entity->GetTransform();
}
