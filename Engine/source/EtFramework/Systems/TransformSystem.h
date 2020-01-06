#pragma once
#include <EtFramework/Components/TransformComponent.h>

#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>


namespace framework {


//---------------------------------
// TransformSystemView
//
// Data required for the transform system to operate
//
struct TransformSystemView final : public ComponentView
{
	TransformSystemView() : ComponentView()
	{
		Declare(parent);
		Declare(transf);
	}

	ParentRead<TransformComponent> parent;
	WriteAccess<TransformComponent> transf;
};

//-----------------
// TransformSystem
//
// Updates transform component world locations respecting the entity hierachy
//
class TransformSystem final : public framework::System<TransformSystem, TransformSystemView>
{
public:
	TransformSystem() = default;

	static void OnComponentAdded(EcsController& controller, TransformComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, TransformComponent& component, T_EntityId const entity);

	void Process(ComponentRange<TransformSystemView>& range) const override;
};


} // namespace framework

