#pragma once
#include <glm\glm.hpp>
#include "AbstractComponent.hpp"

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

	void SetActive();
	bool IsActive() const { return m_IsActive; }

	glm::mat4 GetView() { return m_View; }
	glm::mat4 GetProj() { return m_Projection; }
	glm::mat4 GetViewInv() { return m_ViewInverse; }
	glm::mat4 GetViewProj() { return m_ViewProjection; }
	glm::mat4 GetViewProjInv() { return m_ViewProjectionInverse; }

protected:

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void DrawForward();

private:
	glm::mat4 m_View, m_Projection, m_ViewInverse, 
		m_ViewProjection, m_ViewProjectionInverse;
	float m_FarPlane, m_NearPlane, m_FOV, m_Size;
	bool m_IsActive, m_PerspectiveProjection;

	friend class AbstractScene;
};

