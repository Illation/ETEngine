#include "stdafx.h"
#include "FreeCamera.h"

#include <EtCore/Reflection/Registration.h>

#include <EtFramework/GUI/GuiExtension.h>
#include <EtFramework/Systems/TransformSystem.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace demo {


//==========================
// Free Camera Component
//==========================


RTTR_REGISTRATION
{
	rttr::registration::class_<FreeCameraComponent>("free camera component");
}

ECS_REGISTER_COMPONENT(FreeCameraComponent);


//======================
// Free Camera System
//======================


// static
float const FreeCameraSystem::s_MoveSpeed = 2.f;
float const FreeCameraSystem::s_Accelleration = 5.f;
float const FreeCameraSystem::s_RotationSpeed = 0.0032f;


//-------------------------------
// FreeCameraSystem::c-tor
//
// system dependencies
//
FreeCameraSystem::FreeCameraSystem()
{
	DeclareDependents<fw::TransformSystem::Compute>(); // update before transform world matrices are updated
}

//------------------------------
// FreeCameraSystem::Process
//
// Update a cameras position according to input
//
void FreeCameraSystem::Process(fw::ComponentRange<FreeCameraSystemView>& range)
{
	// common variables
	core::InputManager* const input = core::InputManager::GetInstance();
	float const dt = core::ContextManager::GetInstance()->GetActiveContext()->time->DeltaTime();

	bool hasInput = false;

	for (FreeCameraSystemView& view : range)
	{
		// movement direction
		bool hasMoveInput = false;
		{
			vec3 move;

			move.x -= (input->GetKeyState(E_KbdKey::A) == E_KeyState::Down) ? 1.0f : 0.0f;
			move.x += (input->GetKeyState(E_KbdKey::D) == E_KeyState::Down) ? 1.0f : 0.0f;

			move.y -= (input->GetKeyState(E_KbdKey::Q) == E_KeyState::Down) ? 1.0f : 0.0f;
			move.y += (input->GetKeyState(E_KbdKey::E) == E_KeyState::Down) ? 1.0f : 0.0f;

			move.z += (input->GetKeyState(E_KbdKey::W) == E_KeyState::Down) ? 1.0f : 0.0f;
			move.z -= (input->GetKeyState(E_KbdKey::S) == E_KeyState::Down) ? 1.0f : 0.0f;

			if (!math::isZero(move))
			{
				move = math::normalize(move);
				hasMoveInput = true;
			}

			//Acceleration
			{
				vec3 const delta = move - view.camera->movement;
				float const acc = s_Accelleration * dt;
				view.camera->movement = view.camera->movement + (delta * math::Clamp01(acc));
			}
		}

		//handle scrolling to change camera speed
		if (hasMoveInput || input->GetMouseButton(E_MouseButton::Left) == E_KeyState::Down)
		{
			hasInput = true;

			float const scroll = input->GetMouseWheelDelta().y;
			if (scroll > 0.0f)
			{
				view.camera->speedMultiplier *= 1.1f;
			}
			else if (scroll < -0.01f)
			{
				view.camera->speedMultiplier *= 0.9f;
			}
		}

		float const currSpeed = s_MoveSpeed * view.camera->speedMultiplier;

		// Calculate the view inverse in the same way the camera would do it
		vec3 const lookAt = view.transform->GetPosition() + view.transform->GetForward();
		mat3 const camMat = math::CreateFromMat4(math::inverse(math::lookAt(view.transform->GetPosition(), lookAt, view.transform->GetUp())));
		vec3 currPos = view.transform->GetPosition() + camMat * view.camera->movement * currSpeed * dt;
		view.transform->SetPosition(currPos);

		//Rotate
		//******
		vec2 const look = (input->GetMouseButton(E_MouseButton::Left) == E_KeyState::Down) ? input->GetMouseMove() : vec2();

		view.camera->totalYaw += -look.x * s_RotationSpeed;
		view.camera->totalPitch = math::Clamp(view.camera->totalPitch, math::PI_DIV2, -math::PI_DIV2); // Make sure we can't do a backflip
		view.camera->totalPitch += -look.y * s_RotationSpeed;

		vec3 const right = view.transform->GetRight();
		vec3 const horizontalRight = math::normalize(vec3(right.x, 0, right.z));
		quat const rot = quat(horizontalRight, view.camera->totalPitch) * quat(vec3::UP, view.camera->totalYaw);
		view.transform->SetRotation(rot);
	}

	// ensure we don't loose input when we mouse over a GUI context while dragging the camera
	fw::UnifiedScene::Instance().GetGuiExtension()->SetInputEnabled(!hasInput);
}


} // namespace demo
} // namespace et
