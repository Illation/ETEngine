#include "stdafx.hpp"
#include "CameraComponent.hpp"
#include "../Base/Settings.hpp"
#include "../Base/Context.hpp"
#include "TransformComponent.hpp"

#include "../Graphics/Frustum.h"

#include <glm/gtc/matrix_transform.hpp>

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
	m_StatViewProj = glm::mat4();
	m_StatViewProjInv = glm::mat4();
	m_pFrustum = new Frustum();
}

CameraComponent::~CameraComponent()
{
	delete m_pFrustum;
}

void CameraComponent::Initialize()
{
}

void CameraComponent::Update()
{
	//Calculate projection
	if (m_PerspectiveProjection)
	{
		m_Projection=glm::perspectiveLH(glm::radians(m_FOV),
			(float)(WINDOW.Width) / (float)WINDOW.Height, m_NearPlane, m_FarPlane);
	}
	else
	{
		float viewWidth = (m_Size>0) ? m_Size * WINDOW.GetAspectRatio() : WINDOW.Width;
		float viewHeight = (m_Size>0) ? m_Size : WINDOW.Height;
		m_Projection = glm::ortho(0.f, viewWidth, viewHeight, 0.f, m_NearPlane, m_FarPlane);
	}
	//Calculate parameters to linearize depthbuffer values
	m_DepthProjA = m_FarPlane / (m_FarPlane - m_NearPlane);
	m_DepthProjB = (-m_FarPlane * m_NearPlane) / (m_FarPlane - m_NearPlane);

	//calculate view
	glm::vec3 worldPos = GetTransform()->GetWorldPosition();
	glm::vec3 lookAt = worldPos+GetTransform()->GetForward();
	glm::vec3 upVec = GetTransform()->GetUp();// glm::vec3(0, 1, 0);//
	m_View = glm::lookAtLH(worldPos, lookAt, upVec);

	//calculate utility
	m_ViewInverse = glm::inverse(m_View);
	m_ViewProjection = m_Projection*m_View;
	m_ViewProjectionInverse = glm::inverse(m_View);
	m_StatViewProj = m_Projection*glm::mat4(glm::mat3(m_View));
	m_StatViewProjInv = glm::inverse(m_StatViewProj);

	//Update general frustum
	if (m_FreezeTimer > 0) m_FreezeTimer -= TIME->DeltaTime();
	if (m_FreezeTimer <= 0 && INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_SPACE))
	{
		m_FreezeTimer = 1;
		m_IsFrustumFrozen = !m_IsFrustumFrozen;
	}
	m_pFrustum->SetCullTransform(glm::mat4());//Frustum will be in world space and objects need to transform themselves
	if(!m_IsFrustumFrozen)m_pFrustum->SetToCamera(this);
	m_pFrustum->Update();
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