#pragma once
#include "AbstractComponent.h"


class Camera;
class Viewport;


class CameraComponent final : public AbstractComponent
{
public:
	CameraComponent();
	~CameraComponent() = default;

	float GetFieldOfView() const { return m_FieldOfView; }
	float GetFarPlane() const { return m_FarPlane; }

	void SetFieldOfView(float fov) { m_FieldOfView = fov; }
	void SetOrthoSize(float size) { m_Size = size; }
	void SetNearClippingPlane(float nearPlane) { m_NearPlane = nearPlane; }
	void SetFarClippingPlane(float farPlane) { m_FarPlane = farPlane; }

	void UsePerspectiveProjection() { m_IsPerspective = true; }
	void UseOrthographicProjection() { m_IsPerspective = false; }

	void PopulateCamera(Camera& target, Viewport const& viewport) const;

protected:

	void Init() override {}
	void Deinit() override {}
	void Update() override {}

private:
	bool m_IsPerspective = true;
	float m_FieldOfView = 45.f; // angle of perspective camera in degrees
	float m_Size = 25.f; // width of orthographic camera

	// depth 
	float m_NearPlane = 0.1f;
	float m_FarPlane = 1000.f;
};

