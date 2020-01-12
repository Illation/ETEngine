#include "stdafx.h"
#include "RigidBodySystem.h"

#include <EtFramework/Physics/BulletETM.h>
#include <EtFramework/Physics/PhysicsWorld.h>
#include <EtFramework/Physics/PhysicsManager.h>
#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace fw {


//=====================
// Rigid Body System 
//=====================


//-----------------------------------
// RigidBodySystem::OnComponentAdded
//
// Register transform components in the physics world when they are added to the ECS
//
void RigidBodySystem::OnComponentAdded(EcsController& controller, RigidBodyComponent& component, T_EntityId const entity)
{
	TransformComponent const& transf = controller.GetComponent<TransformComponent>(entity);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	ET_ASSERT((!component.m_IsDynamic) || (component.m_Mass > 0.f));

	// initial transform matches transform component
	btTransform startTransform;
	startTransform.setOrigin(ToBtVec3(transf.GetPosition()));
	startTransform.setRotation(ToBtQuat(transf.GetRotation()));

	// initial inertia
	btVector3 localInertia(0, 0, 0);
	if (component.m_IsDynamic)
	{
		component.m_CollisionShape->calculateLocalInertia(component.m_Mass, localInertia);
	}

	// create rigid body
	component.m_Body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(component.m_Mass, 
		new btDefaultMotionState(startTransform),
		component.m_CollisionShape, 
		localInertia));

	// add to world
	component.m_Body->setUserIndex(-1);
	UnifiedScene::Instance().GetPhysicsWorld().GetWorld()->addRigidBody(component.m_Body);
}

//-------------------------------------
// RigidBodySystem::OnComponentRemoved
//
// Remove respectively
//
void RigidBodySystem::OnComponentRemoved(EcsController& controller, RigidBodyComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	UnifiedScene::Instance().GetPhysicsWorld().GetWorld()->removeRigidBody(component.m_Body);

	delete component.m_Body->getMotionState();
	delete component.m_Body;
	component.m_Body = nullptr;
}

//--------------------------
// RigidBodySystem::Process
//
// Synchronize physics transforms
//  - if the rigid body was externally transformed, it moves the rigid body, otherwise the rigid body moves the transform
//
void RigidBodySystem::Process(ComponentRange<RigidBodySystemView>& range) 
{
	for (RigidBodySystemView& view : range)
	{
		// #todo: split non dynamic rigid bodies into a new component type, e.g ColliderComponent
		if (!(view.rigidBody->m_IsDynamic))
		{
			continue;
		}

		// extract bullet transform
		//--------------------------
		ET_ASSERT(view.rigidBody->m_Body != nullptr);

		btMotionState* const motionState = view.rigidBody->m_Body->getMotionState();
		ET_ASSERT(motionState != nullptr);

		btTransform rbTransform;
		motionState->getWorldTransform(rbTransform);

		bool modified = false;

		// #todo: deal with transform hierachy

		// deal with translations
		//--------------------------
		if (view.transf->HasTranslationChanged())
		{
			rbTransform.setOrigin(ToBtVec3(view.transf->GetPosition()));
			modified = true;
		}
		else
		{
			view.transf->SetPosition(ToEtmVec3(rbTransform.getOrigin())); // will make flags dirty
		}

		// deal with rotations
		//--------------------------
		if (view.transf->HasRotationChanged())
		{
			rbTransform.setRotation(ToBtQuat(view.transf->GetRotation()));
			modified = true;
		}
		else
		{
			view.transf->SetRotation(ToEtmQuat(rbTransform.getRotation())); // will also make flags dirty
		}

		// write back transform changes to rigid body
		//--------------------------------------------
		if (modified)
		{
			motionState->setWorldTransform(rbTransform);
		}
	}
}


} // namespace fw
