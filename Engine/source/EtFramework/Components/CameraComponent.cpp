#include "stdafx.h"
#include "CameraComponent.h"

#include <rttr/registration>

#include <EtRendering/GraphicsTypes/Camera.h>
#include <EtRendering/GraphicsContext/Viewport.h>


// deprecated
//------------

CameraComponent::CameraComponent() = default;

void CameraComponent::PopulateCamera(Camera& target, Viewport const& viewport) const
{
	TransformComponent const* const tfComp = GetTransform();
	target.SetTransformation(tfComp->GetPosition(), tfComp->GetForward(), tfComp->GetUp(), true);

	target.SetIsPerspective(m_IsPerspective, true);
	target.SetFieldOfView(m_FieldOfView, true);
	target.SetSize(m_Size, true);
	target.SetClippingPlanes(m_NearPlane, m_FarPlane, true);
	target.SetViewport(&viewport, true);

	target.Recalculate();
}


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::CameraComponent>("camera component");

	registration::class_<fw::CameraComponentDesc>("camera comp desc")
		.constructor<fw::CameraComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("is perspective", &fw::CameraComponentDesc::isPerspective)
		.property("field of view", &fw::CameraComponentDesc::fieldOfView)
		.property("ortho size", &fw::CameraComponentDesc::size)
		.property("near plane", &fw::CameraComponentDesc::nearPlane)
		.property("far plane", &fw::CameraComponentDesc::farPlane);

	rttr::type::register_converter_func([](fw::CameraComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::CameraComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::CameraComponent);


namespace fw {


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

	target.SetIsPerspective(m_IsPerspective, true);
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
