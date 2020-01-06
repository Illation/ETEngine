#pragma once
#include <EtFramework/Components/LightComponent.h>

#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>


namespace fw {


//------------------
// LightSystemView
//
// Data required for the transform system to operate
//
struct LightSystemView final : public ComponentView
{
	LightSystemView() : ComponentView()
	{
		Declare(light);
	}

	WriteAccess<LightComponent> light;
};

//-------------
// LightSystem
//
// Extracts light colors into the rendering scene representation
//
class LightSystem final : public fw::System<LightSystem, LightSystemView>
{
public:
	LightSystem();

	static void OnComponentAdded(EcsController& controller, LightComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, LightComponent& component, T_EntityId const entity);

	void Process(ComponentRange<LightSystemView>& range) const override;
};


} // namespace fw

