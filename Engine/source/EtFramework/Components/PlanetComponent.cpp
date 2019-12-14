#include "stdafx.h"
#include "PlanetComponent.h"

#include "TransformComponent.h"

#include <EtFramework/SceneGraph/SceneManager.h>


//==================
// Planet Component
//==================


//---------------------------------
// PlanetComponent::c-tor
//
// default constructor sets the parameters
//
PlanetComponent::PlanetComponent(render::PlanetParams const& params)
	: m_Params(params)
{ }

//---------------------------------
// PlanetComponent::Init
//
// Add the planet geometry to the render sccene
//
void PlanetComponent::Init()
{
	TransformComponent* const transform = GetTransform();
	transform->SetRotation(GetTransform()->GetRotation() * quat(vec3(0.0f, 1.0f, 0.0f), etm::radians(270.f)));

	// add planet to scene
	m_PlanetId = SceneManager::GetInstance()->GetRenderScene().AddPlanet(m_Params, transform->GetNodeId());
}

//---------------------------------
// PlanetComponent::Deinit
//
void PlanetComponent::Deinit()
{
	if (m_PlanetId != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().RemovePlanet(m_PlanetId);
	}
}

//---------------------------------
// PlanetComponent::Update
//
// May rotate the planet
//
void PlanetComponent::Update()
{
	TransformComponent* const transform = GetTransform();
	transform->SetPosition(0, 0, 0);

	if (InputManager::GetInstance()->GetKeyState(E_KbdKey::R) == E_KeyState::Pressed)
	{
		m_Rotate = !m_Rotate;
	}

	if (m_Rotate)
	{
		transform->SetRotation(transform->GetRotation() * quat(vec3::UP, -(TIME->DeltaTime() * 0.01f)));
	}
}
