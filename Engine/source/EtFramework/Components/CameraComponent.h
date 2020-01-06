#pragma once
#include "AbstractComponent.h"

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


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


namespace framework {


class TransformComponent;


//---------------------------------
// CameraComponent
//
// Component that describes a view into a scene
//
class CameraComponent final 
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT
		
	// construct destruct
	//--------------------
public:
	CameraComponent() = default;
	~CameraComponent() = default;

	// accessors
	//-----------
	float GetFieldOfView() const { return m_FieldOfView; }
	float GetFarPlane() const { return m_FarPlane; }

	// modifiers
	//-----------
	void SetFieldOfView(float fov) { m_FieldOfView = fov; }
	void SetOrthoSize(float size) { m_Size = size; }
	void SetNearClippingPlane(float nearPlane) { m_NearPlane = nearPlane; }
	void SetFarClippingPlane(float farPlane) { m_FarPlane = farPlane; }

	void UsePerspectiveProjection() { m_IsPerspective = true; }
	void UseOrthographicProjection() { m_IsPerspective = false; }

	void PopulateCamera(Camera& target, Viewport const& viewport, TransformComponent const& tfComp) const;

	// Data
	///////

private:
	bool m_IsPerspective = true;
	float m_FieldOfView = 45.f; // angle of perspective camera in degrees
	float m_Size = 25.f; // width of orthographic camera

	// depth 
	float m_NearPlane = 0.1f;
	float m_FarPlane = 1000.f;
};


//---------------------------------
// CameraComponentDesc
//
// Descriptor for serialization and deserialization of camera components
//
class CameraComponentDesc final : public ComponentDescriptor<CameraComponent>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<CameraComponent>)

	// construct destruct
	//--------------------
public:
	CameraComponentDesc() : ComponentDescriptor<CameraComponent>() {}
	~CameraComponentDesc() = default;

	// ComponentDescriptor interface
	//-------------------------------
	CameraComponent* MakeData() override;

	// Data
	///////

	bool isPerspective = true;
	float fieldOfView = 45.f; 
	float size = 25.f; 
	float nearPlane = 0.1f;
	float farPlane = 1000.f;
};


} // namespace framework

