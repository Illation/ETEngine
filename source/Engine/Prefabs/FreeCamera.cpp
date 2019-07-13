#include "stdafx.h"
#include "FreeCamera.h"

#include <Engine/Components/CameraComponent.hpp>

#include <algorithm>


FreeCamera::FreeCamera()
	:m_Move(vec3(0))
{
}


FreeCamera::~FreeCamera()
{
}

void FreeCamera::Initialize()
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
	if (m_pCamera->IsActive())
	{
		//move
		//*******
		vec3 move = vec3(0, 0, 0);

		move.x = INPUT->IsKeyboardKeyDown('A') ? -1.0f : 0.0f;
		if (move.x == 0) move.x = (INPUT->IsKeyboardKeyDown('D') ? 1.0f : 0.0f);
		move.y = INPUT->IsKeyboardKeyDown('Q') ? 1.0f : 0.0f;
		if (move.y == 0) move.y = -(INPUT->IsKeyboardKeyDown('E') ? 1.0f : 0.0f);
		move.z = INPUT->IsKeyboardKeyDown('W') ? 1.0f : 0.0f; 
		if (move.z == 0) move.z = -(INPUT->IsKeyboardKeyDown('S') ? 1.0f : 0.0f);  

		//Acceleration
		vec3 delta = move - m_Move;
		m_Move = m_Move + delta * m_Acceleration * TIME->DeltaTime();

		//handle scrolling to change camera speed
		if (!etm::nearEqualsV(move, vec3::ZERO) || INPUT->IsMouseButtonDown(1))
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
		vec3 currPos = TRANSFORM->GetPosition() + etm::CreateFromMat4( m_pCamera->GetViewInv() ) * m_Move * currSpeed * TIME->DeltaTime();
		TRANSFORM->SetPosition(currPos);

		//Rotate
		//******
		vec2 look = vec2(0, 0);
		if (INPUT->IsMouseButtonDown(1)) { look = INPUT->GetMouseMovement(); }  

		m_TotalYaw += -look.x * m_RotationSpeed;
		m_TotalPitch = etm::Clamp(m_TotalPitch, etm::PI_DIV2, -etm::PI_DIV2);//Make sure we can't do a backflip
		m_TotalPitch += -look.y * m_RotationSpeed;

		vec3 right = TRANSFORM->GetRight();
		vec3 horizontalRight = etm::normalize( vec3( right.x, 0, right.z ) );
		quat rot = quat( horizontalRight, m_TotalPitch ) * quat( vec3::UP, m_TotalYaw );
		TRANSFORM->SetRotation(rot);
	}
}