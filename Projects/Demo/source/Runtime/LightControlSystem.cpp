#include "stdafx.h"
#include "LightControlSystem.h"

#include <EtFramework/Systems/TransformSystem.h>


namespace demo {


//======================
// Light Control System
//======================


//-------------------------------
// LightControlSystem::c-tor
//
LightControlSystem::LightControlSystem()
{
	DeclareDependents<fw::TransformSystem::Compute>(); // update before transform matrix recalculation
}

//------------------------------
// LightControlSystem::Process
//
void LightControlSystem::Process(fw::ComponentRange<LightControlSystemView>& range)
{
	// common vars
	InputManager* const input = InputManager::GetInstance();
	float const dt = ContextManager::GetInstance()->GetActiveContext()->time->DeltaTime();

	// since input is likely to be rarer than the entity count in the range, we check it once and iterate multiple times
	
	// move about
	if (input->GetKeyState(E_KbdKey::KP_2) == E_KeyState::Down)
	{
		for (LightControlSystemView& view : range)
		{
			view.transf->Rotate(quat(vec3(1.f, 0.f, 0.f), dt));
		}
	}

	if (input->GetKeyState(E_KbdKey::KP_8) == E_KeyState::Down)
	{
		for (LightControlSystemView& view : range)
		{
			view.transf->Rotate(quat(vec3(1.f, 0.f, 0.f), -dt));
		}
	}

	if (input->GetKeyState(E_KbdKey::KP_4) == E_KeyState::Down)
	{
		for (LightControlSystemView& view : range)
		{
			view.transf->Rotate(quat(vec3(0.f, 1.f, 0.f), dt));
		}
	}

	if (input->GetKeyState(E_KbdKey::KP_6) == E_KeyState::Down)
	{
		for (LightControlSystemView& view : range)
		{
			view.transf->Rotate(quat(vec3(0.f, 1.f, 0.f), -dt));
		}
	}

	// #note: this could be optimized by splitting this system in two: one updates the transform and the other the brightness
	// Change light settings
	if (input->GetKeyState(E_KbdKey::KP_3) == E_KeyState::Down)
	{
		for (LightControlSystemView& view : range)
		{
			float const b = view.light->GetBrightness();
			float const nB = b * 4.f;
			view.light->SetBrightness(b - (nB - b) * dt);
			LOG("Linear: " + std::to_string(view.light->GetBrightness()));
		}
	}

	if (input->GetKeyState(E_KbdKey::KP_9) == E_KeyState::Down)
	{
		for (LightControlSystemView& view : range)
		{
			float const b = view.light->GetBrightness();
			float const nB = b * 4.f;
			view.light->SetBrightness(b + (nB - b) * dt);
			LOG("Linear: " + std::to_string(view.light->GetBrightness()));
		}
	}
}


} // namespace demo

