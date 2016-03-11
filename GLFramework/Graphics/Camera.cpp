#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "../Base/Settings.hpp"
#include "../Base/Context.hpp"

#define WINDOW Settings::GetInstance()->Window
#define CONTEXT Context::GetInstance()

Camera::Camera()
{
	//Nothing to do
}

Camera::~Camera()
{
	//Nothing to do
}

void Camera::Initialize()
{
	//Nothing to do
}

void Camera::Update()
{
	if (m_PerspectiveProjection)
	{
		m_Projection=glm::perspective(glm::radians(m_FOV),
			(float)(WINDOW.Width) / (float)WINDOW.Height, m_NearPlane, m_FarPlane);
	}
	else
	{
		float viewWidth = (m_Size>0) ? m_Size * WINDOW.AspectRatio : WINDOW.Width;
		float viewHeight = (m_Size>0) ? m_Size : WINDOW.Height;
		m_Projection = glm::ortho(0.f, viewWidth, viewHeight, 0.f, m_NearPlane, m_FarPlane);
	}

	m_View = glm::lookAt(
		glm::vec3(2.5f, 2.0f, 2.5f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
		);

	m_ViewInverse = glm::inverse(m_View);
	m_ViewProjection = m_Projection*m_View;
	m_ViewProjectionInverse = glm::inverse(m_View);
}

void Camera::SetActive()
{
	CONTEXT->pCamera = this;
}