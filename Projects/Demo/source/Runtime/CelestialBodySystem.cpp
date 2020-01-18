#include "stdafx.h"
#include "CelestialBodySystem.h"

#include <EtFramework/Systems/TransformSystem.h>


namespace demo {


//=======================
// Celestial Body System
//=======================


//----------------------------
// CelestialBodySystem::c-tor
//
CelestialBodySystem::CelestialBodySystem()
{
	DeclareDependents<fw::TransformSystem::Compute>(); // update before transform matrix recalculation
}

//------------------------------
// CelestialBodySystem::Process
//
void CelestialBodySystem::Process(fw::ComponentRange<CelestialBodySystemView>& range)
{
	// common variables
	bool const toggle = (InputManager::GetInstance()->GetKeyState(E_KbdKey::R) == E_KeyState::Pressed);
	float const dt = ContextManager::GetInstance()->GetActiveContext()->time->DeltaTime();

	for (CelestialBodySystemView& view : range)
	{
		if (toggle)
		{
			view.body->isRotating = !(view.body->isRotating);
		}

		if (view.body->isRotating)
		{
			view.transf->Rotate(quat(vec3::UP, dt * view.body->rotationSpeed));
		}
	}
}


} // namespace demo