#include "stdafx.h"
#include "FixedCamera.h"

#include <EtFramework/Components/CameraComponent.h>


FixedCamera::FixedCamera()
{
}


FixedCamera::~FixedCamera()
{
}

void FixedCamera::Init()
{
	//Camera Component
	m_pCamera = new CameraComponent();
	AddComponent(m_pCamera);

	GetTransform()->SetPosition(2.5, -2, -2.5f);

	GetTransform()->RotateEuler( 0, etm::PI_DIV4, 0 );
	GetTransform()->RotateEuler( etm::PI_DIV4 * 0.41f, 0, etm::PI_DIV4 * 0.41f );
}