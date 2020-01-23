#include "stdafx.h"
#include "SpawnSystem.h"

#include <EtFramework/Systems/RigidBodySystem.h>
#include <EtFramework/Components/RigidBodyComponent.h>
#include <EtFramework/Components/ModelComponent.h>


namespace et {
namespace demo {


//======================
// Spawn System
//======================


//--------------------
// SpawnSystem::c-tor
//
// system dependencies
//
SpawnSystem::SpawnSystem()
{
	DeclareDependents<fw::RigidBodySystem>(); // update before rigid bodies so bullet simulates our spheres as soon as they are added
}

//------------------------------
// SpawnSystem::Process
//
// Spawn upon user input
//
void SpawnSystem::Process(fw::ComponentRange<SpawnSystemView>& range)
{
	if (!(core::InputManager::GetInstance()->GetMouseButton(E_MouseButton::Right) >= E_KeyState::Down))
	{
		return;
	}

	// common variables
	fw::EcsCommandBuffer& cb = GetCommandBuffer();
	float const dt = core::ContextManager::GetInstance()->GetActiveContext()->time->DeltaTime();

	for (SpawnSystemView& view : range)
	{
		view.spawner->cooldown -= dt;
		if (view.spawner->cooldown <= 0.f)
		{
			view.spawner->cooldown = view.spawner->interval;

			fw::T_EntityId const spawned = cb.AddEntity();

			vec3 const& dir = view.transform->GetForward();

			fw::TransformComponent tf;
			tf.SetPosition(view.transform->GetWorldPosition() + dir * view.spawner->scale);
			tf.SetScale(vec3(view.spawner->scale));

			fw::RigidBodyComponent rb(true, view.spawner->mass, view.spawner->collisionShape);

			fw::ModelComponent model(view.spawner->mesh, view.spawner->material);
			
			cb.AddComponents(spawned, tf, model, rb);

			vec3 const impulse = dir * view.spawner->impulse;
			cb.OnMerge(spawned, fw::EcsCommandBuffer::T_OnMergeFn([impulse](fw::EcsController& ecs, fw::T_EntityId const entity)
				{
					ecs.GetComponent<fw::RigidBodyComponent>(entity).ApplyImpulse(impulse);
				}));
		}
	}
}


} // namespace demo
} // namespace et
