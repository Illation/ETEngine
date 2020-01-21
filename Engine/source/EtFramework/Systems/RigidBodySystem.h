#pragma once
#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/Components/RigidBodyComponent.h>

#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>


namespace et {
namespace fw {


//---------------------------------
// RigidBodySystemView
//
// Data required for the rigid body system to operate
//
struct RigidBodySystemView final : public ComponentView
{
	RigidBodySystemView() : ComponentView()
	{
		Declare(rigidBody);
		Declare(transf);
	}

	WriteAccess<RigidBodyComponent> rigidBody;
	WriteAccess<TransformComponent> transf;
};

//-----------------
// RigidBodySystem
//
// Makes sure transform components are moved or can move rigid body components
//
class RigidBodySystem final : public fw::System<RigidBodySystem, RigidBodySystemView>
{
public:
	RigidBodySystem() = default;

	static void OnComponentAdded(EcsController& controller, RigidBodyComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, RigidBodyComponent& component, T_EntityId const entity);

	void Process(ComponentRange<RigidBodySystemView>& range) override;
};


} // namespace fw
} // namespace et

