#include "stdafx.h"
#include "TransformSystem.h"

#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace fw {


//=====================
// Transform System 
//=====================


//-----------------------------------
// TransformSystem::OnComponentAdded
//
// Register transform components in the render scene when they are added to the ECS
//
void TransformSystem::OnComponentAdded(EcsController& controller, TransformComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	component.m_NodeId = UnifiedScene::Instance().GetRenderScene().AddNode(component.GetWorld());
}

//-------------------------------------
// TransformSystem::OnComponentRemoved
//
// Remove respectively
//
void TransformSystem::OnComponentRemoved(EcsController& controller, TransformComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	UnifiedScene::Instance().GetRenderScene().RemoveNode(component.GetNodeId());
}

//--------------------------
// TransformSystem::Process
//
// Hierachically update
//
void TransformSystem::Process(ComponentRange<TransformSystemView>& range) const
{
	render::Scene& renderScene = UnifiedScene::Instance().GetRenderScene();

	for (TransformSystemView& view : range)
	{
		// this is the local matrix
		view.transf->m_WorldTransform = etm::scale(view.transf->m_Scale) * etm::rotate(view.transf->m_Rotation) * etm::translate(view.transf->m_Position);

		if (view.parent.IsValid())
		{
			// transform based on parent matrix
			view.transf->m_WorldTransform = view.transf->m_WorldTransform * view.parent->m_WorldTransform;

			// update world variables
			view.transf->m_WorldPosition = (view.parent->m_WorldTransform * vec4(view.transf->m_Position, 0)).xyz;
			view.transf->m_WorldRotation = view.parent->GetWorldRotation() * view.transf->m_WorldRotation;
			view.transf->m_WorldScale = view.parent->GetWorldScale() * view.transf->m_Scale;
		}
		else
		{
			view.transf->m_WorldPosition = view.transf->m_Position;
			view.transf->m_WorldRotation = view.transf->m_Rotation;
			view.transf->m_WorldScale = view.transf->m_Scale;
		}

		// orientation helpers
		view.transf->m_Forward = view.transf->m_WorldRotation * vec3::FORWARD;
		view.transf->m_Right = view.transf->m_WorldRotation * vec3::RIGHT;
		view.transf->m_Up = etm::cross(view.transf->m_Forward, view.transf->m_Right);

		// update in the rendering scene
		renderScene.UpdateNode(view.transf->m_NodeId, view.transf->m_WorldTransform);

		// reset
		view.transf->m_TransformChanged = TransformComponent::E_TransformChanged::None;
	}
}


} // namespace fw
