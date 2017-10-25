#include "stdafx.hpp"
#include "FreeCamera.hpp"
#include "../Components/CameraComponent.hpp"
#include <gtx/matrix_decompose.hpp>
#include "../Helper/MathHelper.hpp"
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
		//HANDLE INPUT
		vec3 move = vec3(0, 0, 0);
		vec2 look = vec2(0, 0);

		move.x = INPUT->IsKeyboardKeyDown('A') ? -1.0f : 0.0f;
		if (move.x == 0) move.x = (INPUT->IsKeyboardKeyDown('D') ? 1.0f : 0.0f);
		move.y = INPUT->IsKeyboardKeyDown('Q') ? 1.0f : 0.0f;
		if (move.y == 0) move.y = -(INPUT->IsKeyboardKeyDown('E') ? 1.0f : 0.0f);
		move.z = INPUT->IsKeyboardKeyDown('W') ? 1.0f : 0.0f; 
		if (move.z == 0) move.z = -(INPUT->IsKeyboardKeyDown('S') ? 1.0f : 0.0f);  

		vec3 delta = move - m_Move;
		m_Move = m_Move + delta * m_Acceleration * TIME->DeltaTime();

		auto currSpeed = m_MoveSpeed; 

		if (!etm::nearEqualsV(move, vec3::ZERO) || INPUT->IsMouseButtonDown(1))
		{
			//not delta time dependant
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
		currSpeed *= m_SpeedMultiplier;  
		if (INPUT->IsMouseButtonDown(1)) { look = INPUT->GetMouseMovement(); }  

		//Get state 
		vec3 forward = GetTransform()->GetForward(); 
		vec3 right = GetTransform()->GetRight(); 
		vec3 up = vec3::UP;
		vec3 currPos = GetTransform()->GetPosition(); 

		//Translate 
		currPos = currPos + forward * m_Move.z * currSpeed * TIME->DeltaTime();
		currPos = currPos + up * m_Move.y * currSpeed * TIME->DeltaTime();
		currPos = currPos + right * m_Move.x * currSpeed * TIME->DeltaTime();
		TRANSFORM->SetPosition(currPos);

		//Rotate
		//linear because its based on the mouse move delta instead of framerate
		float rotSpeed = m_RotationSpeed*0.016f;
		m_TotalYaw += -look.x * rotSpeed;
		m_TotalPitch = std::max(m_TotalPitch, -1.57f);
		m_TotalPitch = std::min(m_TotalPitch, 1.57f);
		m_TotalPitch += look.y * rotSpeed;
		quat rot = quat( vec3::UP, m_TotalYaw );
		rot = rot * quat( etm::normalize(vec3(right.x, 0, right.z)), m_TotalPitch );
		//rot = etm::rotate(rot, etm::normalize(vec3(right.x, right.y, 0)), m_TotalPitch);
		//rot = etm::rotate(rot, forward, etm::PI);
		TRANSFORM->SetRotation(rot);
	}
}