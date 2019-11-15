#include "stdafx.h"
#include "PlanetTemplate.h"

#include <Engine/Components/TransformComponent.h>
#include <Engine/Components/CameraComponent.h>


void PlanetTemplate::Initialize()
{
	GetTransform()->SetRotation(GetTransform()->GetRotation() * quat(vec3(0.0f, 1.0f, 0.0f), etm::radians(270.f)));

	//LoadTextures
	LoadPlanet();

	// add planet to scene
}

void PlanetTemplate::Update()
{
	GetTransform()->SetPosition(0, 0, 0);

	if (INPUT->GetKeyState(E_KbdKey::R) == E_KeyState::Pressed)
	{
		m_Rotate = !m_Rotate;
	}

	if (m_Rotate)
	{
		GetTransform()->SetRotation(GetTransform()->GetRotation() * quat(vec3::UP, -(TIME->DeltaTime() * 0.01f)));
	}
}


void PlanetTemplate::SetSunlight(core::T_SlotId const lightId)
{
	// access render::planet
	// get atmo ID
	// set sunlight on render::atmo
}

