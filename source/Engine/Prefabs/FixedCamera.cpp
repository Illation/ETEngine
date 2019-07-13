#include "stdafx.h"
#include "FixedCamera.h"

#include <Engine/Components/CameraComponent.hpp>


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

	GetTransform()->SetPosition(2.5, -2, -2.5f);

	GetTransform()->RotateEuler( 0, etm::PI_DIV4, 0 );
	GetTransform()->RotateEuler( etm::PI_DIV4 * 0.41f, 0, etm::PI_DIV4 * 0.41f );
}