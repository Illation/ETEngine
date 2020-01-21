#include "stdafx.h"
#include "ModelInit.h"

#include <EtRendering/MaterialSystem/MaterialInterface.h>

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace fw {


//============
// Model Init
//============


//-----------------------------------
// ModelInit::OnComponentAdded
//
// Register models in the render scene when they are added to the ECS
//
void ModelInit::OnComponentAdded(EcsController& controller, ModelComponent& component, T_EntityId const entity)
{
	render::I_Material const* const mat = component.m_Material.get_as<render::I_Material>();

	component.m_InstanceId = UnifiedScene::Instance().GetRenderScene().AddInstance(mat,
		component.m_Mesh, 
		controller.GetComponent<TransformComponent>(entity).GetNodeId());
}

//-------------------------------------
// ModelInit::OnComponentRemoved
//
// Remove respectively
//
void ModelInit::OnComponentRemoved(EcsController& controller, ModelComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	UnifiedScene::Instance().GetRenderScene().RemoveInstance(component.m_InstanceId);
}


} // namespace fw
} // namespace et
