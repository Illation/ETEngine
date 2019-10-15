#pragma once
#include "AbstractComponent.h"

class Frustum;

class CameraComponent : public AbstractComponent
{
public:
	CameraComponent();
	virtual ~CameraComponent();

	void SetFieldOfView(float fov) { m_FOV = fov; }
	void SetOrthoSize(float size) { m_Size = size; }
	void SetNearClippingPlane(float nearPlane) { m_NearPlane = nearPlane; }
	void SetFarClippingPlane(float farPlane) { m_FarPlane = farPlane; }

	void UsePerspectiveProjection() { m_PerspectiveProjection = true; }
	void UseOrthographicProjection() { m_PerspectiveProjection = false; }

	mat4 GetView() { return m_View; }
	mat4 GetProj() { return m_Projection; }
	mat4 GetViewInv() { return m_ViewInverse; }
	mat4 GetViewProj() { return m_ViewProjection; }
	mat4 GetViewProjInv() { return m_ViewProjectionInverse; }
	mat4 GetStatViewProj() { return m_StatViewProj; }
	mat4 GetStatViewProjInv() { return m_StatViewProjInv; }

	float GetDepthProjA() { return m_DepthProjA; }
	float GetDepthProjB() { return m_DepthProjB; }

	float GetNearPlane() { return m_NearPlane; }
	float GetFarPlane() { return m_FarPlane; }
	float GetFOV() { return m_FOV; }

	Frustum* GetFrustum() { return m_pFrustum; }

protected:

	virtual void Initialize() {}
	virtual void Update();
	virtual void Draw() {}
	virtual void DrawForward() {}

private:
	mat4 m_View, m_Projection, m_ViewInverse, 
		m_ViewProjection, m_ViewProjectionInverse,
		m_StatViewProj, m_StatViewProjInv;//Ignore camera position for gbuffer reconstruction and skybox
	float m_FarPlane, m_NearPlane, m_FOV, m_Size;
	float m_DepthProjA, m_DepthProjB;//parameters to linearize depth values
	bool m_PerspectiveProjection;

	Frustum* m_pFrustum = nullptr;

	bool m_IsFrustumFrozen = false;
	float m_FreezeTimer = 0;

	friend class AbstractScene;
};

