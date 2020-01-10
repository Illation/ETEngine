#pragma once
#include <EtFramework/Components/PlanetComponent.h>
#include <EtFramework/Components/AtmosphereComponent.h>
#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/Components/TransformComponent.h>

#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>


namespace fw {


//---------------------------------
// PlanetCameraLinkSystemView
//
// Data required for the planet camera link system to process
//
struct PlanetCameraLinkSystemView final : public ComponentView
{
	PlanetCameraLinkSystemView() : ComponentView()
	{
		Declare(camera);
		Declare(cameraTransform);
		Declare(link);

		Declare(planet);
		Declare(planetTransform);
		Declare(planetAtmo);
	}

	WriteAccess<CameraComponent> camera;
	ReadAccess<TransformComponent> cameraTransform;
	ReadAccess<PlanetCameraLinkComponent> link;

	EntityRead<PlanetComponent> planet;
	EntityRead<TransformComponent> planetTransform;
	EntityRead<AtmosphereComponent> planetAtmo;
};

//------------------------
// PlanetCameraLinkSystem
//
// Sets the clipping planes of a camera relative to a planet
//
class PlanetCameraLinkSystem final : public System<PlanetCameraLinkSystem, PlanetCameraLinkSystemView>
{
public:
	PlanetCameraLinkSystem();

	void Process(ComponentRange<PlanetCameraLinkSystemView>& range) const override;
};


} // namespace fw

