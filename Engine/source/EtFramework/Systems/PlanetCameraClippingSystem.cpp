#include "stdafx.h"
#include "PlanetCameraClippingSystem.h"

#include <EtFramework/Systems/TransformSystem.h>


namespace et {
namespace fw {


//============================
// Planet Camera Link System 
//============================


//-------------------------------
// PlanetCameraLinkSystem::c-tor
//
// system dependencies
//
PlanetCameraLinkSystem::PlanetCameraLinkSystem()
{
	DeclareDependencies<TransformSystem::Compute>(); // update lights after updating transforms, though we don't need to wait for flags to update
}

//---------------------------------
// PlanetCameraLinkSystem::Process
//
// Set the cameras clipping planes to the planets size
//
void PlanetCameraLinkSystem::Process(ComponentRange<PlanetCameraLinkSystemView>& range) 
{
	for (PlanetCameraLinkSystemView& view : range)
	{
		PlanetComponent const* const planet = view.planet[view.link->planet];
		TransformComponent const* const planetTransform = view.planetTransform[view.link->planet];
		AtmosphereComponent const* const planetAtmo = view.planetAtmo[view.link->planet];

		ET_ASSERT(planet != nullptr);
		ET_ASSERT(planetTransform != nullptr);
		ET_ASSERT(planetAtmo != nullptr);

		float const planetRad = planet->GetRadius();

		float const radius = std::max(planetRad + planet->GetMaxHeight(), planetRad + planetAtmo->GetAtmosphereHeight());
		float const altitude = math::distance(planetTransform->GetPosition(), view.cameraTransform->GetPosition()) - planetRad;

		view.camera->SetFarClippingPlane((sqrtf(powf(planetRad + altitude, 2) - powf(planetRad, 2)) + sqrtf(powf(radius, 2) - powf(planetRad, 2))) * 10);
		view.camera->SetNearClippingPlane(view.camera->GetFarPlane() * 0.000003f);
	}
}


} // namespace fw
} // namespace et
