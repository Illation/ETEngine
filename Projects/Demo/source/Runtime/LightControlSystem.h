#pragma once
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/Components/LightComponent.h>

#include <Common/ControledLightComponent.h>


namespace et {
namespace demo {


//---------------------------
// LightControlSystemView
//
// ECS access pattern for the light control system
//
struct LightControlSystemView final : public fw::ComponentView
{
	LightControlSystemView() : fw::ComponentView()
	{
		Declare(light);
		Declare(transf);
		Include<ControledLightComponent>();
	}

	WriteAccess<fw::LightComponent> light;
	WriteAccess<fw::TransformComponent> transf;
};

//---------------------------
// LightControlSystem
//
// Allow the user to change a light in the 3D scene
//
class LightControlSystem final : public fw::System<LightControlSystem, LightControlSystemView>
{
public:
	LightControlSystem();

	void Process(fw::ComponentRange<LightControlSystemView>& range) override;
};


} // namespace demo
} // namespace et

