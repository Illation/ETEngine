#include "stdafx.hpp"
#include "CameraComponent.hpp"
#include "../Base/Settings.hpp"
#include "../Base/Context.hpp"
#include "TransformComponent.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define WINDOW Settings::GetInstance()->Window
#define CONTEXT Context::GetInstance()

CameraComponent::CameraComponent():
	m_FOV(45),
	m_NearPlane(0.1f),
	m_FarPlane(1000.0f),
	m_Size(25.0f),
	m_PerspectiveProjection(true),
m_IsActive(true)
{
	m_Projection = glm::mat4();
	m_View = glm::mat4();
	m_ViewInverse = glm::mat4();
	m_ViewProjection = glm::mat4();
	m_ViewProjectionInverse = glm::mat4();
}

CameraComponent::~CameraComponent()
{
	//Nothing to do
}

void CameraComponent::Initialize()
{
	//Nothing to do
}

void CameraComponent::Update()
{
	if (m_PerspectiveProjection)
	{
		m_Projection=glm::perspectiveLH(glm::radians(m_FOV),
			(float)(WINDOW.Width) / (float)WINDOW.Height, m_NearPlane, m_FarPlane);
	}
	else
	{
		float viewWidth = (m_Size>0) ? m_Size * WINDOW.AspectRatio : WINDOW.Width;
		float viewHeight = (m_Size>0) ? m_Size : WINDOW.Height;
		m_Projection = glm::ortho(0.f, viewWidth, viewHeight, 0.f, m_NearPlane, m_FarPlane);
	}

	glm::vec3 worldPos = GetTransform()->GetWorldPosition();
	glm::vec3 lookAt = worldPos+GetTransform()->GetForward();
	glm::vec3 upVec = GetTransform()->GetUp();// glm::vec3(0, 1, 0);//
	m_View = glm::lookAtLH(worldPos, lookAt, upVec);

	m_ViewInverse = glm::inverse(m_View);
	m_ViewProjection = m_Projection*m_View;
	m_ViewProjectionInverse = glm::inverse(m_View);
}

void CameraComponent::Draw()
{
	//Nothing to do
}
void CameraComponent::DrawForward()
{
	//Nothing to do
}

void CameraComponent::SetActive()
{
	CONTEXT->pCamera = this;
}