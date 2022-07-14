#include "stdafx.h"
#include "LightSystem.h"

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Systems/TransformSystem.h>


namespace et {
namespace fw {


//===============
// Light System 
//===============


//--------------------
// LightSystem::c-tor
//
// system dependencies
//
LightSystem::LightSystem()
{
	DeclareDependencies<TransformSystem::Compute>(); // update lights after updating transforms, though we don't need to wait for flags to update
}

//-----------------------------------
// LightSystem::OnComponentAdded
//
// Register lights in the render scene when they are added to the ECS
//
void LightSystem::OnComponentAdded(EcsController& controller, LightComponent& component, T_EntityId const entity)
{
	vec3 const col = component.m_Color * component.m_Brightness;

	component.m_LightId = UnifiedScene::Instance().GetRenderScene().AddLight(col,
		controller.GetComponent<TransformComponent>(entity).GetNodeId(),
		component.m_Type == LightComponent::E_Type::Directional,
		component.m_CastsShadow);
}

//-------------------------------------
// LightSystem::OnComponentRemoved
//
// Remove respectively
//
void LightSystem::OnComponentRemoved(EcsController& controller, LightComponent& component, T_EntityId const entity)
{
	ET_UNUSED(controller);
	ET_UNUSED(entity);

	UnifiedScene::Instance().GetRenderScene().RemoveLight(component.m_LightId);
}

//--------------------------
// LightSystem::Process
//
// Extract light colors
//
void LightSystem::Process(ComponentRange<LightSystemView>& range) 
{
	render::Scene& renderScene = UnifiedScene::Instance().GetRenderScene();

	for (LightSystemView& view : range)
	{
		if (view.light->m_ColorChanged)
		{
			vec3 const col = view.light->m_Color * view.light->m_Brightness;
			renderScene.UpdateLightColor(view.light->m_LightId, col);

			view.light->m_ColorChanged = false;
		}
	}
}


} // namespace fw
} // namespace et
