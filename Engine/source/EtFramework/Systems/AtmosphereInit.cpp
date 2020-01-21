#include "stdafx.h"
#include "AtmosphereInit.h"

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace fw {


//=================
// Atmopshere Init
//=================


//-----------------------------------
// AtmosphereInit::OnComponentAdded
//
// Register atmospheres in the render scene when they are added to the ECS
//
void AtmosphereInit::OnComponentAdded(EcsController& controller, AtmosphereComponent& component, T_EntityId const entity)
{
	render::AtmosphereInstance atmoInst;
	atmoInst.atmosphereId = component.m_AssetId;
	atmoInst.nodeId = controller.GetComponent<TransformComponent>(entity).GetNodeId();
	atmoInst.height = component.m_Height;
	atmoInst.groundRadius = component.m_GroundHeight;

	component.m_RenderId = UnifiedScene::Instance().GetRenderScene().AddAtmosphere(atmoInst);
}

//-------------------------------------
// AtmosphereInit::OnComponentRemoved
//
// Remove respectively
//
void AtmosphereInit::OnComponentRemoved(EcsController& controller, AtmosphereComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	if (component.m_RenderId != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetRenderScene().RemoveAtmosphere(component.m_RenderId);
	}
}


} // namespace fw
} // namespace et
