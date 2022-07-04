#include "stdafx.h"
#include "CameraSyncSystem.h"

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Systems/TransformSystem.h>


namespace et {
namespace fw {


//====================
// Camera Sync System 
//====================


//--------------------
// CameraSyncSystem::c-tor
//
// system dependencies
//
CameraSyncSystem::CameraSyncSystem()
{
	DeclareDependencies<TransformSystem::Compute>(); // update cameras after updating transforms, though we don't need to wait for flags to update
}

//-----------------------------------
// CameraSyncSystem::OnComponentAdded
//
// Register cameras in the render scene when they are added to the ECS
//
void CameraSyncSystem::OnComponentAdded(EcsController& controller, CameraComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	component.m_Id = UnifiedScene::Instance().GetRenderScene().AddCamera();
}

//-------------------------------------
// CameraSyncSystem::OnComponentRemoved
//
// Remove respectively
//
void CameraSyncSystem::OnComponentRemoved(EcsController& controller, CameraComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	UnifiedScene::Instance().GetRenderScene().RemoveCamera(component.m_Id);
}

//--------------------------
// CameraSyncSystem::Process
//
// Update camera parameters
//
void CameraSyncSystem::Process(ComponentRange<CameraSyncSystemView>& range)
{
	render::Scene& renderScene = UnifiedScene::Instance().GetRenderScene();

	for (CameraSyncSystemView& view : range)
	{
		render::Camera& target = renderScene.GetCamera(view.camera->m_Id);

		target.SetTransformation(view.transf->GetPosition(), view.transf->GetForward(), view.transf->GetUp(), true);

		target.SetIsPerspective(true, true); // #todo: support ortho cameras
		target.SetFieldOfView(view.camera->m_FieldOfView, true);
		target.SetSize(view.camera->m_Size, true);
		target.SetClippingPlanes(view.camera->m_NearPlane, view.camera->m_FarPlane, true);
		target.SetViewport(view.camera->m_Viewport, true);

		target.Recalculate();
	}
}


} // namespace fw
} // namespace et


