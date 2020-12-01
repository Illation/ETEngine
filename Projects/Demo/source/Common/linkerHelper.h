#pragma once
#include <EtFramework/linkerHelper.h>

#include "SpawnComponent.h"
#include "ControledLightComponent.h"
#include "SwirlyLightComponent.h"
#include "CelestialBodyComponent.h"
#include "PlaylistComponent.h"


namespace et {
namespace demo {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	fw::ForceLinking();

	FORCE_LINKING(SpawnComponentDesc)
	FORCE_LINKING(ControledLightComponent)
	FORCE_LINKING(SwirlyLightComponentDesc)
	FORCE_LINKING(CelestialBodyComponent)
	FORCE_LINKING(PlaylistComponent)
}


} // namespace demo
} // namespace et


