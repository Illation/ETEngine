#include "stdafx.hpp"
#include "OrbitCamera.hpp"
#include "../Components/CameraComponent.hpp"

#include "FixedCamera.hpp"

OrbitCamera::OrbitCamera()
{
}


OrbitCamera::~OrbitCamera()
{
}

void OrbitCamera::Initialize()
{
	//Camera Component
	m_pCamera = new CameraComponent();
	AddComponent(m_pCamera);

	m_pFixCam = new FixedCamera();
	m_pCamParent = new Entity();
	m_pCamParent->AddChild(m_pFixCam);
	AddChild(m_pCamParent);
}

void OrbitCamera::Start()
{
	m_pCamera = m_pFixCam->GetComponent<CameraComponent>();
	GetScene()->SetActiveCamera(m_pCamera);
	CAMERA->SetFarClippingPlane(5000);
}

void OrbitCamera::SetRotation(float pitch, float yaw)
{
	m_TotalPitch = pitch;
	m_TotalYaw = yaw;
}

void OrbitCamera::Update()
{
	if (m_pCamera->IsActive())
	{
		//TODO
		//Scrolling changes distance
		m_pFixCam->TRANSFORM->SetPosition(0, 0, m_Distance);

		auto camTf = m_pCamParent->TRANSFORM;
		auto dT = TIME->DeltaTime();

		camTf->SetPosition(m_RotationCenter);
		vec3 move = vec3(0, 0, 0);
		vec2 look = vec2(0, 0);
		if (INPUT->IsMouseButtonDown(SDL_BUTTON_MIDDLE))
		{
			auto mouseMove = INPUT->GetMouseMovement();
			move.z = mouseMove.y;
			move.x = -mouseMove.x;
		}
		else if (INPUT->IsMouseButtonDown(SDL_BUTTON_LEFT))
		{
			look.x = INPUT->GetMouseMovement().x;
			look.y = INPUT->GetMouseMovement().y;
		}
		m_TotalYaw = look.x*m_RotationSpeed*dT;
		m_TotalPitch = look.y*m_RotationSpeed*dT;
		TRANSFORM->RotateEuler(m_TotalPitch, m_TotalYaw, 0);
		//auto camRot = camTf->GetRotation();
		//camTf->Rotate(-camRot);

		auto currSpeed = 50;
		vec3 forward = camTf->GetForward();
		vec3 right = camTf->GetRight();
		vec3 up = camTf->GetUp();
		vec3 currPos = TRANSFORM->GetPosition();
		currPos = currPos + forward*(move.y*currSpeed*dT);
		currPos = currPos + right*(move.x*currSpeed*dT);
		currPos = currPos + up*(move.z*currSpeed*dT);
		TRANSFORM->SetPosition(currPos);
	}
}