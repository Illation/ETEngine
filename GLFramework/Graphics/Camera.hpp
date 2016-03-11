#pragma once
#include <glm\glm.hpp>

class Camera
{
public:
	Camera();
	~Camera();

	void SetFieldOfView(float fov) { m_FOV = fov; }
	void SetOrthoSize(float size) { m_Size = size; }
	void SetNearClippingPlane(float nearPlane) { m_NearPlane = nearPlane; }
	void SetFarClippingPlane(float farPlane) { m_FarPlane = farPlane; }

	void UsePerspectiveProjection() { m_PerspectiveProjection = true; }
	void UseOrthographicProjection() { m_PerspectiveProjection = false; }

	void SetActive();
	bool IsActive() const { return m_IsActive; }

	void Initialize();
	void Update();

	glm::mat4 GetView() { return m_View; }
	glm::mat4 GetProj() { return m_Projection; }
	glm::mat4 GetViewInv() { return m_ViewInverse; }
	glm::mat4 GetViewProj() { return m_ViewProjection; }
	glm::mat4 GetViewProjInv() { return m_ViewProjectionInverse; }

private:
	glm::mat4 m_View, m_Projection, m_ViewInverse, 
		m_ViewProjection, m_ViewProjectionInverse;
	float m_FarPlane, m_NearPlane, m_FOV, m_Size;
	bool m_IsActive, m_PerspectiveProjection;
};

