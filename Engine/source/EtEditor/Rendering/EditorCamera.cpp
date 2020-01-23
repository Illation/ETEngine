#include "stdafx.h"
#include "EditorCamera.h"

#include <rttr/registration>

#include <EtRendering/GraphicsTypes/Camera.h>

#include <EtFramework/Systems/TransformSystem.h>


namespace et {
namespace edit {


//==========================
// Editor Camera Component
//==========================


RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<EditorCameraComponent>("editor camera component");
}

ECS_REGISTER_COMPONENT(EditorCameraComponent);


//======================
// Editor Camera System
//======================


// static
float const EditorCameraSystem::s_MoveSpeed = 2.f;
float const EditorCameraSystem::s_Accelleration = 5.f;
float const EditorCameraSystem::s_RotationSpeed = 0.0032f;


//-------------------------------
// EditorCameraSystem::c-tor
//
// system dependencies
//
EditorCameraSystem::EditorCameraSystem()
{
	DeclareDependents<fw::TransformSystem::Compute>(); // update before transform world matrices are updated
}

//------------------------------
// AudioListenerSystem::Process
//
// Update a cameras position according to input
//
void EditorCameraSystem::Process(fw::ComponentRange<EditorCameraSystemView>& range) 
{
	// common variables
	core::InputManager* const input = core::InputManager::GetInstance();
	float const dt = core::ContextManager::GetInstance()->GetActiveContext()->time->DeltaTime();

	for (EditorCameraSystemView& view : range)
	{
		// movement direction
		vec3 move;

		if (view.camera->isEnabled)
		{
			move.x -= (input->GetKeyState(E_KbdKey::A) == E_KeyState::Down) ? 1.0f : 0.0f;
			move.x += (input->GetKeyState(E_KbdKey::D) == E_KeyState::Down) ? 1.0f : 0.0f;

			move.y -= (input->GetKeyState(E_KbdKey::Q) == E_KeyState::Down) ? 1.0f : 0.0f;
			move.y += (input->GetKeyState(E_KbdKey::E) == E_KeyState::Down) ? 1.0f : 0.0f;

			move.z += (input->GetKeyState(E_KbdKey::W) == E_KeyState::Down) ? 1.0f : 0.0f;
			move.z -= (input->GetKeyState(E_KbdKey::S) == E_KeyState::Down) ? 1.0f : 0.0f;
		}

		if (!etm::isZero(move))
		{
			move = etm::normalize(move);
		}

		//Acceleration
		{
			vec3 const delta = move - view.camera->movement;
			float const acc = s_Accelleration * dt;
			view.camera->movement = view.camera->movement + (delta * etm::Clamp01(acc));
		}

		//handle scrolling to change camera speed
		float const scroll = view.camera->isEnabled ? input->GetMouseWheelDelta().y : 0.f;
		if (scroll > 0.0f)
		{
			view.camera->speedMultiplier *= 1.1f;
		}
		else if (scroll < -0.01f)
		{
			view.camera->speedMultiplier *= 0.9f;
		}

		float const currSpeed = s_MoveSpeed * view.camera->speedMultiplier;

		//move relative to cameras view space - luckily the camera already has those inverted matrices calculated
		mat3 const camMat = etm::CreateFromMat4(view.camera->renderCamera->GetViewInv());
		view.transform->SetPosition(view.transform->GetPosition() + camMat * view.camera->movement * currSpeed * dt);

		//Rotate
		//******
		vec2 const look = view.camera->isEnabled ? input->GetMouseMove() : vec2();

		view.camera->totalYaw += -look.x * s_RotationSpeed;
		view.camera->totalPitch = etm::Clamp(view.camera->totalPitch, etm::PI_DIV2, -etm::PI_DIV2); // Make sure we can't do a backflip
		view.camera->totalPitch += -look.y * s_RotationSpeed;

		vec3 const right = view.transform->GetRight();
		vec3 const horizontalRight = etm::normalize(vec3(right.x, 0, right.z));
		quat const rot = quat(horizontalRight, view.camera->totalPitch) * quat(vec3::UP, view.camera->totalYaw);
		view.transform->SetRotation(rot);
	}
}


} // namespace edit
} // namespace et
