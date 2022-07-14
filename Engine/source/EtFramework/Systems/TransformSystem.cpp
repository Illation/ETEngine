#include "stdafx.h"
#include "TransformSystem.h"

#include "RigidBodySystem.h"

#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
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
	ET_UNUSED(controller);
	ET_UNUSED(entity);

	component.m_NodeId = UnifiedScene::Instance().GetRenderScene().AddNode(component.GetWorld());
}

//-------------------------------------
// TransformSystem::OnComponentRemoved
//
// Remove respectively
//
void TransformSystem::OnComponentRemoved(EcsController& controller, TransformComponent& component, T_EntityId const entity)
{
	ET_UNUSED(controller);
	ET_UNUSED(entity);

	UnifiedScene::Instance().GetRenderScene().RemoveNode(component.GetNodeId());
}

//---------------------------------
// TransformSystem::Compute::c-tor
//
// dependency setup
//
TransformSystem::Compute::Compute()
{
	DeclareDependencies<RigidBodySystem>(); // the rigid body system may update transformations

	DeclareDependents<TransformSystem::Reset>();
}

//-----------------------------------
// TransformSystem::Compute::Process
//
// Update transforms
//
void TransformSystem::Compute::Process(ComponentRange<TransformSystem::ComputeView>& range) 
{
	render::Scene& renderScene = UnifiedScene::Instance().GetRenderScene();

	for (ComputeView& view : range)
	{
		// if neither this component nor the parent component has an updated transform, we don't need to recalculate anything
		if (!view.transf->HasTransformChanged() && (!view.parent.IsValid() || !view.parent->HasTransformChanged()))
		{
			continue;
		}

		// this is the local matrix
		view.transf->m_WorldTransform = math::scale(view.transf->m_Scale) 
			* math::rotate(view.transf->m_Rotation) 
			* math::translate(view.transf->m_Position);

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
		view.transf->m_Up = math::cross(view.transf->m_Forward, view.transf->m_Right);

		// update in the rendering scene
		renderScene.UpdateNode(view.transf->m_NodeId, view.transf->m_WorldTransform);

		// since we changed our transform we need to set the transform changed flags so that the change trickles down to children
		view.transf->m_TransformChanged = TransformComponent::E_TransformChanged::All;
	}
}

//---------------------------------
// TransformSystem::Reset::Process
//
// Reset dirty flag
//
void TransformSystem::Reset::Process(ComponentRange<TransformSystem::ResetView>& range) 
{
	for (ResetView& view : range)
	{
		view.transf->m_TransformChanged = TransformComponent::E_TransformChanged::None;
	}
}


} // namespace fw
} // namespace et
