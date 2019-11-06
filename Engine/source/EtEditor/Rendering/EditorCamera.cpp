#include "stdafx.h"
#include "EditorCamera.h"

#include <Engine/Graphics/Camera.h>
#include <Engine/Components/CameraComponent.h>
#include <Engine/Components/TransformComponent.h>


//================
// Editor Camera
//================


// static
float const EditorCamera::s_MoveSpeed = 2.f;
float const EditorCamera::s_Accelleration = 5.f;
float const EditorCamera::s_RotationSpeed = 0.0032f;


//---------------------------------
// EditorCamera::c-tor
//
EditorCamera::EditorCamera()
	: m_Position(0, -1, -3.5)
	, m_Forward(vec3::FORWARD)
	, m_Right(vec3::RIGHT)
	, m_Up(vec3::UP)
{ }

//---------------------------------
// EditorCamera::Update
//
// Move the camera about based on input - only one viewports camera should be enabled at a time, we assume the mouse is clicked
//
void EditorCamera::Update(Camera const& currentCamera)
{
	Time* time = nullptr;
	{
		BaseContext* baseContext = ContextManager::GetInstance()->GetActiveContext();
		if (baseContext == nullptr)
		{
			return;
		}

		time = baseContext->time;
	}

	//move
	//*******
	vec3 move;

	if (m_IsEnabled)
	{
		move.x -= (InputManager::GetInstance()->GetKeyState(E_KbdKey::A) == E_KeyState::Down) ? 1.0f : 0.0f;
		move.x += (InputManager::GetInstance()->GetKeyState(E_KbdKey::D) == E_KeyState::Down) ? 1.0f : 0.0f;

		move.y -= (InputManager::GetInstance()->GetKeyState(E_KbdKey::Q) == E_KeyState::Down) ? 1.0f : 0.0f;
		move.y += (InputManager::GetInstance()->GetKeyState(E_KbdKey::E) == E_KeyState::Down) ? 1.0f : 0.0f;

		move.z += (InputManager::GetInstance()->GetKeyState(E_KbdKey::W) == E_KeyState::Down) ? 1.0f : 0.0f;
		move.z -= (InputManager::GetInstance()->GetKeyState(E_KbdKey::S) == E_KeyState::Down) ? 1.0f : 0.0f;
	}

	if (!etm::isZero(move))
	{
		move = etm::normalize(move);
	}

	//Acceleration
	{
		vec3 const delta = move - m_Movement;
		float const acc = s_Accelleration * time->DeltaTime();
		m_Movement = m_Movement + (delta * etm::Clamp01(acc));
	}

	//handle scrolling to change camera speed
	float const scroll = m_IsEnabled ? InputManager::GetInstance()->GetMouseWheelDelta().y : 0.f;
	if (scroll > 0.0f)
	{
		m_SpeedMultiplier *= 1.1f;
	}
	else if (scroll < -0.01f)
	{
		m_SpeedMultiplier *= 0.9f;
	}

	float const currSpeed = s_MoveSpeed * m_SpeedMultiplier;

	//move relative to cameras view space - luckily the camera already has those inverted matrices calculated
	mat3 const camMat = etm::CreateFromMat4(currentCamera.GetViewInv());
	m_Position = m_Position + camMat * m_Movement * currSpeed * time->DeltaTime();

	//Rotate
	//******
	vec2 const look = m_IsEnabled ? InputManager::GetInstance()->GetMouseMove() : vec2(0.f);

	m_TotalYaw += -look.x * s_RotationSpeed;
	m_TotalPitch = etm::Clamp(m_TotalPitch, etm::PI_DIV2, -etm::PI_DIV2);//Make sure we can't do a backflip
	m_TotalPitch += -look.y * s_RotationSpeed;

	quat const rot = quat(etm::normalize(vec3(m_Right.x, 0, m_Right.z)), m_TotalPitch) * quat(vec3::UP, m_TotalYaw);

	// update transformation
	m_Forward = rot * vec3::FORWARD;
	m_Right = rot * vec3::RIGHT;
	m_Up = etm::cross(m_Forward, m_Right);
}

//---------------------------------
// EditorCamera::PopulateCamera
//
// Populate the parameters of a graphical camera for rendering 
//
void EditorCamera::PopulateCamera(Camera& target, Viewport const* const viewport) const
{
	target.SetTransformation(m_Position, m_Forward, m_Up, true);

	target.SetIsPerspective(true, true);
	target.SetFieldOfView(m_FieldOfView, true);
	target.SetClippingPlanes(m_NearPlane, m_FarPlane, true);
	target.SetViewport(viewport, true);

	target.Recalculate();
}

//---------------------------------
// EditorCamera::ImitateComponent
//
// Position the camera in a way that matches the set up of a runtime camera
//
void EditorCamera::ImitateComponent(CameraComponent const* const comp)
{
	TransformComponent const* const tfComp = comp->GetTransform();
	m_Position = tfComp->GetPosition();
	m_Forward = tfComp->GetForward();
	m_Up = tfComp->GetUp();
	m_Right = tfComp->GetRight();

	m_Movement = vec3(0.f);

	m_FieldOfView = comp->GetFieldOfView();
}

