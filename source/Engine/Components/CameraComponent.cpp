#include "stdafx.hpp"
#include "CameraComponent.hpp"
#include "../Base/Settings.hpp"
#include "../Base/Context.hpp"
#include "TransformComponent.hpp"

#include "../Graphics/Frustum.hpp"

#include <gtc/matrix_transform.hpp>

#define CONTEXT Context::GetInstance()

CameraComponent::CameraComponent():
	m_FOV(45),
	m_NearPlane(0.1f),
	m_FarPlane(1000.0f),
	m_Size(25.0f),
	m_PerspectiveProjection(true),
m_IsActive(true)
{
	m_Projection = mat4();
	m_View = mat4();
	m_ViewInverse = mat4();
	m_ViewProjection = mat4();
	m_ViewProjectionInverse = mat4();
	m_StatViewProj = mat4();
	m_StatViewProjInv = mat4();
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
		m_Projection=etm::perspective(etm::radians(m_FOV),
			(float)(WINDOW.Width) / (float)WINDOW.Height, m_NearPlane, m_FarPlane);
	}
	else
	{
		float viewWidth = (m_Size>0) ? m_Size * WINDOW.GetAspectRatio() : WINDOW.Width;
		float viewHeight = (m_Size>0) ? m_Size : WINDOW.Height;
		m_Projection = etm::orthographic(0.f, viewWidth, viewHeight, 0.f, m_NearPlane, m_FarPlane);
	}
	//Calculate parameters to linearize depthbuffer values
	m_DepthProjA = m_FarPlane / (m_FarPlane - m_NearPlane);
	m_DepthProjB = (-m_FarPlane * m_NearPlane) / (m_FarPlane - m_NearPlane);

	//calculate view
	vec3 worldPos = GetTransform()->GetWorldPosition();
	vec3 lookAt = worldPos+GetTransform()->GetForward();
	vec3 upVec = GetTransform()->GetUp();// vec3(0, 1, 0);//
	m_View = etm::lookAt(worldPos, lookAt, upVec);

	//calculate utility
	m_ViewInverse = etm::inverse(m_View);
	m_ViewProjection = m_View * m_Projection;
	m_ViewProjectionInverse = etm::inverse(m_View);
	m_StatViewProj = etm::DiscardW(m_View) * m_Projection;
	m_StatViewProjInv = inverse(m_StatViewProj);

	//Update general frustum
	if (m_FreezeTimer > 0) m_FreezeTimer -= TIME->DeltaTime();
	if (m_FreezeTimer <= 0 && INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_SPACE))
	{
		m_FreezeTimer = 1;
		m_IsFrustumFrozen = !m_IsFrustumFrozen;
	}
	m_pFrustum->SetCullTransform(mat4());//Frustum will be in world space and objects need to transform themselves
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