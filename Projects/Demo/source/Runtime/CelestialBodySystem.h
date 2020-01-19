#pragma once
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/TransformComponent.h>

#include <Common/CelestialBodyComponent.h>


namespace et {
namespace demo {


//---------------------------
// CelestialBodySystemView
//
// ECS access pattern for the light control system
//
struct CelestialBodySystemView final : public fw::ComponentView
{
	CelestialBodySystemView() : fw::ComponentView()
	{
		Declare(body);
		Declare(transf);
	}

	WriteAccess<CelestialBodyComponent> body;
	WriteAccess<fw::TransformComponent> transf;
};

//---------------------------
// CelestialBodySystem
//
// Controls the movement of celestial bodies
//
class CelestialBodySystem final : public fw::System<CelestialBodySystem, CelestialBodySystemView>
{
public:
	CelestialBodySystem();

	void Process(fw::ComponentRange<CelestialBodySystemView>& range) override;
};


} // namespace demo
} // namespace et

