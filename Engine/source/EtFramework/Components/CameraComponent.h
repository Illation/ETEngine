#pragma once
#include <EtFramework/SceneGraph/ComponentDescriptor.h>


// fwd
namespace et { namespace render {
	class Camera;
	class Viewport;
}
	REGISTRATION_NS(fw);
namespace fw {
	class TransformComponent;
} }


namespace et {
namespace fw {


//---------------------------------
// CameraComponent
//
// Component that describes a view into a scene
//
class CameraComponent final : public SimpleComponentDescriptor
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	RTTR_ENABLE(SimpleComponentDescriptor) // for serialization
	REGISTRATION_FRIEND_NS(fw)
	DECLARE_FORCED_LINKING()

	friend class CameraSyncSystem;
		
	// construct destruct
	//--------------------
public:
	CameraComponent() = default;
	~CameraComponent() = default;

	// accessors
	//-----------
	core::T_SlotId GetId() const { return m_Id; }

	Ptr<rhi::Viewport> GetViewport() const { return m_Viewport; }

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

	void SetViewport(Ptr<rhi::Viewport> const viewport);

	// Data
	///////

private:
	core::T_SlotId m_Id = core::INVALID_SLOT_ID;

	Ptr<rhi::Viewport> m_Viewport;

	bool m_IsPerspective = true;
	float m_FieldOfView = 45.f; // angle of perspective camera in degrees
	float m_Size = 25.f; // width of orthographic camera

	// depth 
	float m_NearPlane = 0.1f;
	float m_FarPlane = 1000.f;
};


} // namespace fw
} // namespace et

