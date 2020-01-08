#include "stdafx.h"
#include "PlanetInit.h"

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace fw {


//=============
// Planet Init
//=============


//-----------------------------------
// PlanetInit::OnComponentAdded
//
// Register planets in the render scene when they are added to the ECS
//
void PlanetInit::OnComponentAdded(EcsController& controller, PlanetComponent& component, T_EntityId const entity)
{
	TransformComponent& transf = controller.GetComponent<TransformComponent>(entity);
	transf.SetRotation(transf.GetRotation() * quat(vec3(0.0f, 1.0f, 0.0f), etm::radians(270.f)));

	component.m_PlanetId = UnifiedScene::Instance().GetRenderScene().AddPlanet(component.m_Params, transf.GetNodeId());
}

//-------------------------------------
// PlanetInit::OnComponentRemoved
//
// Remove respectively
//
void PlanetInit::OnComponentRemoved(EcsController& controller, PlanetComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	if (component.m_PlanetId != core::INVALID_SLOT_ID)
	{
		UnifiedScene::Instance().GetRenderScene().RemovePlanet(component.m_PlanetId);
	}
}


} // namespace fw
