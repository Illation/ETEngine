#include "stdafx.h"
#include "CameraComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtRendering/GraphicsTypes/Camera.h>
#include <EtRendering/GraphicsContext/Viewport.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(CameraComponent, "camera component")
		.property("is perspective", &CameraComponent::m_IsPerspective)
		.property("field of view", &CameraComponent::m_FieldOfView)
		.property("ortho size", &CameraComponent::m_Size)
		.property("near plane", &CameraComponent::m_NearPlane)
		.property("far plane", &CameraComponent::m_FarPlane)
	END_REGISTER_CLASS_POLYMORPHIC(CameraComponent, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(CameraComponent);


//==================
// Camera Component 
//==================


//---------------------------------
// CameraComponent::PopulateCamera
//
// fill out a render camera from the component
//
void CameraComponent::PopulateCamera(render::Camera& target, render::Viewport const& viewport, TransformComponent const& tfComp) const
{
	target.SetTransformation(tfComp.GetPosition(), tfComp.GetForward(), tfComp.GetUp(), true);

	target.SetIsPerspective(true, true); // #todo: support ortho cameras
	target.SetFieldOfView(m_FieldOfView, true);
	target.SetSize(m_Size, true);
	target.SetClippingPlanes(m_NearPlane, m_FarPlane, true);
	target.SetViewport(&viewport, true);

	target.Recalculate();
}


} // namespace fw
} // namespace et
