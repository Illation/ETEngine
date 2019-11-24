#include "stdafx.h"
#include "FreeCamera.h"

#include <algorithm>

#include <EtFramework/Components/CameraComponent.h>


FreeCamera::FreeCamera()
	:m_Move(vec3(0))
{
}

void FreeCamera::Init()
{
	//Camera Component
	m_pCamera = new CameraComponent();
	AddComponent(m_pCamera);
}

void FreeCamera::SetRotation(float pitch, float yaw)
{
	m_TotalPitch = pitch;
	m_TotalYaw = yaw;
}

void FreeCamera::Update()
{
	if (m_pCamera == CAMERA)
	{
		//move
		//*******
		vec3 move = vec3(0, 0, 0);


		move.x = (INPUT->GetKeyState(E_KbdKey::A) == E_KeyState::Down) ? -1.0f : 0.0f;
		if (move.x == 0)
		{
			move.x = (INPUT->GetKeyState(E_KbdKey::D) == E_KeyState::Down) ? 1.0f : 0.0f;
		}
		move.y = (INPUT->GetKeyState(E_KbdKey::Q) == E_KeyState::Down) ? -1.0f : 0.0f;
		if (move.y == 0)
		{
			move.y = (INPUT->GetKeyState(E_KbdKey::E) == E_KeyState::Down) ? 1.0f : 0.0f;
		}
		move.z = (INPUT->GetKeyState(E_KbdKey::W) == E_KeyState::Down) ? 1.0f : 0.0f;
		if (move.z == 0)
		{
			move.z = (INPUT->GetKeyState(E_KbdKey::S) == E_KeyState::Down) ? -1.0f : 0.0f;
		}

		//Acceleration
		vec3 delta = move - m_Move;
		m_Move = m_Move + delta * m_Acceleration * TIME->DeltaTime();

		//handle scrolling to change camera speed
		if (!etm::nearEqualsV(move, vec3::ZERO) || INPUT->GetMouseButton(E_MouseButton::Left) == E_KeyState::Down)
		{
			float scroll = INPUT->GetMouseWheelDelta().y;
			if (scroll > 0.0f)
			{
				m_SpeedMultiplier *= 1.1f;
			}
			else if (scroll < -0.01f)
			{
				m_SpeedMultiplier *= 0.9f;
			}
		}

		float currSpeed = m_MoveSpeed * m_SpeedMultiplier;  

		//move relative to cameras view space - luckily the camera already has those inverted matrices calculated
		vec3 const lookAt = TRANSFORM->GetPosition() + TRANSFORM->GetForward();
		mat3 const camMat = etm::CreateFromMat4(etm::inverse(etm::lookAt(TRANSFORM->GetPosition(), lookAt, TRANSFORM->GetUp())));
		vec3 currPos = TRANSFORM->GetPosition() + camMat * m_Move * currSpeed * TIME->DeltaTime();
		TRANSFORM->SetPosition(currPos);

		//Rotate
		//******
		vec2 look = vec2(0, 0);
		if (INPUT->GetMouseButton(E_MouseButton::Left) == E_KeyState::Down) 
		{ 
			look = INPUT->GetMouseMove();
		}

		m_TotalYaw += -look.x * m_RotationSpeed;
		m_TotalPitch = etm::Clamp(m_TotalPitch, etm::PI_DIV2, -etm::PI_DIV2);//Make sure we can't do a backflip
		m_TotalPitch += -look.y * m_RotationSpeed;

		vec3 right = TRANSFORM->GetRight();
		vec3 horizontalRight = etm::normalize( vec3( right.x, 0, right.z ) );
		quat rot = quat( horizontalRight, m_TotalPitch ) * quat( vec3::UP, m_TotalYaw );
		TRANSFORM->SetRotation(rot);
	}
}