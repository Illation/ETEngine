#include "stdafx.hpp"
#include "FreeCamera.hpp"
#include "../Components/CameraComponent.hpp"
#include <gtx/matrix_decompose.hpp>
#include "../Helper/MathHelper.hpp"
#include <algorithm>

FreeCamera::FreeCamera()
	:m_Move(glm::vec3(0))
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
		glm::vec3 move = glm::vec3(0, 0, 0);
		glm::vec2 look = glm::vec2(0, 0);

		move.x = INPUT->IsKeyboardKeyDown('A') ? -1.0f : 0.0f;
		if (move.x == 0) move.x = (INPUT->IsKeyboardKeyDown('D') ? 1.0f : 0.0f);
		move.y = INPUT->IsKeyboardKeyDown('Q') ? 1.0f : 0.0f;
		if (move.y == 0) move.y = -(INPUT->IsKeyboardKeyDown('E') ? 1.0f : 0.0f);
		move.z = INPUT->IsKeyboardKeyDown('W') ? 1.0f : 0.0f; 
		if (move.z == 0) move.z = -(INPUT->IsKeyboardKeyDown('S') ? 1.0f : 0.0f);  

		glm::vec3 delta = move - m_Move;
		m_Move += delta * m_Acceleration * TIME->DeltaTime();

		auto currSpeed = m_MoveSpeed; 

		if (!(move == glm::vec3(0)) || INPUT->IsMouseButtonDown(1))
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
		glm::vec3 forward = GetTransform()->GetForward(); 
		glm::vec3 right = GetTransform()->GetRight(); 
		glm::vec3 up = glm::vec3(0, 1, 0); 
		glm::vec3 currPos = GetTransform()->GetPosition(); 

		//Translate 
		currPos += forward * m_Move.z * currSpeed * TIME->DeltaTime();
		currPos += up * m_Move.y * currSpeed * TIME->DeltaTime();
		currPos += right * m_Move.x * currSpeed * TIME->DeltaTime();
		TRANSFORM->SetPosition(currPos);

		//Rotate
		//linear because its based on the mouse move delta instead of framerate
		float rotSpeed = m_RotationSpeed*0.016f;
		m_TotalYaw += -look.x * rotSpeed;
		m_TotalPitch = std::max(m_TotalPitch, -1.57f);
		m_TotalPitch = std::min(m_TotalPitch, 1.57f);
		m_TotalPitch += look.y * rotSpeed;
		glm::quat rot = glm::quat(0, 0, 0, 1);
		rot = glm::rotate(rot, m_TotalYaw, glm::vec3(0, 1, 0));
		rot = glm::rotate(rot, m_TotalPitch, glm::normalize(glm::vec3(right.x, 0, right.z)));
		//rot = glm::rotate(rot, m_TotalPitch, glm::normalize(glm::vec3(right.x, right.y, 0)));
		//rot = glm::rotate(rot, 3.1415f, forward);
		TRANSFORM->SetRotation(rot);
	}
}