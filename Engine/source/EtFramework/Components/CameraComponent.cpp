#include "stdafx.h"
#include "CameraComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtRendering/GraphicsTypes/Camera.h>
#include <EtRendering/GraphicsContext/Viewport.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace fw {


//==================
// Camera Component 
//==================


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
DEFINE_FORCED_LINKING(CameraComponent)


//---------------------------------
// CameraComponent::SetViewport
//
// Update viewport on render camera immediately for loading order purposes
//
void CameraComponent::SetViewport(Ptr<render::Viewport> const viewport)
{
	m_Viewport = viewport;
	if (m_Id != core::INVALID_SLOT_ID)
	{
		fw::UnifiedScene::Instance().GetRenderScene().GetCamera(m_Id).SetViewport(viewport, true);
	}
}


} // namespace fw
} // namespace et
