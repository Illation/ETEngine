#include "stdafx.h"
#include "CameraComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtRendering/GraphicsTypes/Camera.h>
#include <EtRendering/GraphicsContext/Viewport.h>


namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<CameraComponent>("camera component");

	BEGIN_REGISTER_POLYMORPHIC_CLASS(CameraComponentDesc, "camera comp desc")
		.property("is perspective", &CameraComponentDesc::isPerspective)
		.property("field of view", &CameraComponentDesc::fieldOfView)
		.property("ortho size", &CameraComponentDesc::size)
		.property("near plane", &CameraComponentDesc::nearPlane)
		.property("far plane", &CameraComponentDesc::farPlane)
	END_REGISTER_POLYMORPHIC_CLASS(CameraComponentDesc, I_ComponentDescriptor);
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
void CameraComponent::PopulateCamera(Camera& target, Viewport const& viewport, TransformComponent const& tfComp) const
{
	target.SetTransformation(tfComp.GetPosition(), tfComp.GetForward(), tfComp.GetUp(), true);

	target.SetIsPerspective(true, true); // #todo: support ortho cameras
	target.SetFieldOfView(m_FieldOfView, true);
	target.SetSize(m_Size, true);
	target.SetClippingPlanes(m_NearPlane, m_FarPlane, true);
	target.SetViewport(&viewport, true);

	target.Recalculate();
}


//=============================
// Camera Component Descriptor
//=============================


//-------------------------------
// CameraComponentDesc::MakeData
//
// Create a camera component from a descriptor
//
CameraComponent* CameraComponentDesc::MakeData()
{
	CameraComponent* const ret = new CameraComponent();

	if (isPerspective)
	{
		ret->UseOrthographicProjection();
	}
	else
	{
		ret->UseOrthographicProjection();
	}

	ret->SetFieldOfView(fieldOfView);
	ret->SetOrthoSize(size);

	ret->SetNearClippingPlane(nearPlane);
	ret->SetFarClippingPlane(farPlane);

	return ret;
}


} // namespace fw
