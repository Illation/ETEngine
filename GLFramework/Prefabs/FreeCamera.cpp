#include "stdafx.hpp"
#include "FreeCamera.hpp"
#include "../Components/CameraComponent.hpp"
#include <glm/gtx/matrix_decompose.hpp>

FreeCamera::FreeCamera()
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
		move.x = INPUT->IsKeyboardKeyDown('A') ? 1.0f : 0.0f;
		if (move.x == 0) move.x = -(INPUT->IsKeyboardKeyDown('D') ? 1.0f : 0.0f);

		move.y = INPUT->IsKeyboardKeyDown('Q') ? 1.0f : 0.0f;
		if (move.y == 0) move.y = -(INPUT->IsKeyboardKeyDown('E') ? 1.0f : 0.0f);

		move.z = INPUT->IsKeyboardKeyDown('W') ? 1.0f : 0.0f;
		if (move.z == 0) move.z = -(INPUT->IsKeyboardKeyDown('S') ? 1.0f : 0.0f);


		auto currSpeed = m_MoveSpeed;
		if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_LSHIFT))
			currSpeed *= m_SpeedMultiplier;

		glm::vec2 look = glm::vec2(0, 0);
		if (INPUT->IsMouseButtonDown(1))
		{
			look = INPUT->GetMouseMovement();
		}

		//CALCULATE TRANSFORMS
		glm::vec3 forward = GetTransform()->GetForward();
		glm::vec3 right = GetTransform()->GetRight();
		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::vec3 currPos = GetTransform()->GetPosition();

		currPos += forward * move.z * currSpeed * TIME->DeltaTime();
		currPos += up * move.y * currSpeed * TIME->DeltaTime();
		currPos += right * move.x * currSpeed * TIME->DeltaTime();

		m_TotalYaw = look.x * m_RotationSpeed * TIME->DeltaTime();
		m_TotalPitch = -look.y * m_RotationSpeed * TIME->DeltaTime();

		GetTransform()->RotateEuler(m_TotalPitch, m_TotalYaw, 0);
		GetTransform()->Translate(currPos);
	}
}