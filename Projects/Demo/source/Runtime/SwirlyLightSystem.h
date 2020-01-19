#pragma once
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/Components/LightComponent.h>

#include <Common/SwirlyLightComponent.h>


namespace et {
namespace demo {


//---------------------------
// SwirlyLightSystemView
//
// ECS access pattern for the swirly light system
//
struct SwirlyLightSystemView final : public fw::ComponentView
{
	SwirlyLightSystemView() : fw::ComponentView()
	{
		Declare(swirly);
		Declare(transf);
		Declare(light);
	}

	WriteAccess<SwirlyLightComponent> swirly;
	WriteAccess<fw::TransformComponent> transf;
	WriteAccess<fw::LightComponent> light;
};

//---------------------------
// SwirlyLightSystem
//
// System that moves lights about the scene in a seemingly random way, akin to fireflies
//
class SwirlyLightSystem final : public fw::System<SwirlyLightSystem, SwirlyLightSystemView>
{
public:
	SwirlyLightSystem();

	void Process(fw::ComponentRange<SwirlyLightSystemView>& range) override;
};


} // namespace demo
} // namespace et

