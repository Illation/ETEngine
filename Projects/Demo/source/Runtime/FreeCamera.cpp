#include "stdafx.h"
#include "FreeCamera.h"

#include <EtFramework/Components/CameraComponent.h>


//=============
// Free Camera
//=============


// static
float const FreeCamera::s_MoveSpeed = 2.5f;
float const FreeCamera::s_RotationSpeed = 0.0032f;
float const FreeCamera::s_Acceleration = 5.f;


//--------------------------
// FreeCamera::c-tor
//
FreeCamera::FreeCamera()
{
	Reset();
}

//--------------------------
// FreeCamera::Reset
//
void FreeCamera::Reset()
{
	m_TotalPitch = 0.f;
	m_TotalYaw = 0.f;
	m_SpeedMultiplier = 1.f;
	m_Move = vec3(0.f);
}

//--------------------------
// FreeCamera::Update
//
// move camera based on user input
//
void FreeCamera::Update()
{
	if (m_Camera == nullptr)
	{
		return;
	}

	TransformComponent* const transform = m_Camera->GetTransform();
	InputManager* const input = InputManager::GetInstance();

	//move
	//*******
	bool hasMoveInput = false;

	{
		vec3 move;

		move.x -= (input->GetKeyState(E_KbdKey::A) == E_KeyState::Down) ? 1.0f : 0.0f;
		move.x += (input->GetKeyState(E_KbdKey::D) == E_KeyState::Down) ? 1.0f : 0.0f;

		move.y -= (input->GetKeyState(E_KbdKey::Q) == E_KeyState::Down) ? 1.0f : 0.0f;
		move.y += (input->GetKeyState(E_KbdKey::E) == E_KeyState::Down) ? 1.0f : 0.0f;

		move.z += (input->GetKeyState(E_KbdKey::W) == E_KeyState::Down) ? 1.0f : 0.0f;
		move.z -= (input->GetKeyState(E_KbdKey::S) == E_KeyState::Down) ? 1.0f : 0.0f;

		if (!etm::isZero(move))
		{
			move = etm::normalize(move);
			hasMoveInput = true;
		}

		//Acceleration
		vec3 const delta = move - m_Move;
		m_Move = m_Move + delta * s_Acceleration * TIME->DeltaTime();
	}

	//handle scrolling to change camera speed
	if (hasMoveInput || input->GetMouseButton(E_MouseButton::Left) == E_KeyState::Down)
	{
		float scroll = input->GetMouseWheelDelta().y;
		if (scroll > 0.0f)
		{
			m_SpeedMultiplier *= 1.1f;
		}
		else if (scroll < -0.01f)
		{
			m_SpeedMultiplier *= 0.9f;
		}
	}

	float const currSpeed = s_MoveSpeed * m_SpeedMultiplier;

	//move relative to cameras view space - luckily the camera already has those inverted matrices calculated
	vec3 const lookAt = transform->GetPosition() + transform->GetForward();
	mat3 const camMat = etm::CreateFromMat4(etm::inverse(etm::lookAt(transform->GetPosition(), lookAt, transform->GetUp())));
	vec3 currPos = transform->GetPosition() + camMat * m_Move * currSpeed * TIME->DeltaTime();
	transform->SetPosition(currPos);

	//Rotate
	//******
	vec2 look = vec2(0, 0);
	if (input->GetMouseButton(E_MouseButton::Left) == E_KeyState::Down)
	{ 
		look = input->GetMouseMove();
	}

	m_TotalYaw += -look.x * s_RotationSpeed;
	m_TotalPitch = etm::Clamp(m_TotalPitch, etm::PI_DIV2, -etm::PI_DIV2);//Make sure we can't do a backflip
	m_TotalPitch += -look.y * s_RotationSpeed;

	vec3 const right = transform->GetRight();
	vec3 const horizontalRight = etm::normalize( vec3( right.x, 0, right.z ) );
	quat const rot = quat( horizontalRight, m_TotalPitch ) * quat( vec3::UP, m_TotalYaw );
	transform->SetRotation(rot);
}
