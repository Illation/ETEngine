#pragma once
#include <EtFramework/Components/TransformComponent.h>

#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>


namespace fw {


//-----------------
// TransformSystem
//
// Updates transform component world locations respecting the entity hierachy
//  - runs in two phases (compute -> reset) in order to skip unchanged components while respecting hierachy needs
//
class TransformSystem final
{
public:
	// Init / Deinit
	//----------------
	static void OnComponentAdded(EcsController& controller, TransformComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, TransformComponent& component, T_EntityId const entity);

	//---------------------------------
	// Compute
	//
	// Updates the positions, rotations, scales and matricies for the transform hierachy
	//

	struct ComputeView final : public ComponentView
	{
		ComputeView() : ComponentView()
		{
			Declare(parent);
			Declare(transf);
		}

		ParentRead<TransformComponent> parent;
		WriteAccess<TransformComponent> transf;
	};

	class Compute final : public fw::System<Compute, ComputeView>
	{
	public:
		Compute();

		void Process(ComponentRange<ComputeView>& range) const override;
	};


	//---------------------------------
	// Reset
	//
	// Resets dirty flags
	//

	struct ResetView final : public ComponentView
	{
		ResetView() : ComponentView()
		{
			Declare(transf);
		}

		WriteAccess<TransformComponent> transf;
	};

	class Reset final : public fw::System<Reset, ResetView>
	{
	public:
		Reset() = default;

		void Process(ComponentRange<ResetView>& range) const override;
	};
};


} // namespace fw

