#include "stdafx.hpp"
#include "FixedCamera.hpp"
#include "../Components/CameraComponent.hpp"
#include <glm/gtx/matrix_decompose.hpp>

FixedCamera::FixedCamera()
{
}


FixedCamera::~FixedCamera()
{
}

void FixedCamera::Initialize()
{
	//Camera Component
	m_pCamera = new CameraComponent();
	AddComponent(m_pCamera);

	GetTransform()->Translate(2.5, -2, -2.5f);

	GetTransform()->RotateEuler(0, glm::radians(45.f), 0);
	GetTransform()->RotateEuler(glm::radians(20.f), 0, glm::radians(20.f));
}