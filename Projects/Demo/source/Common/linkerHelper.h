#pragma once
#include <EtFramework/linkerHelper.h>

#include "SpawnComponent.h"
#include "ControledLightComponent.h"
#include "SwirlyLightComponent.h"
#include "CelestialBodyComponent.h"
#include "PlaylistComponent.h"


namespace demo {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	::ForceLinking();

	FORCE_LINKING(SpawnComponentDesc)
	FORCE_LINKING(ControledLightComponentDesc)
	FORCE_LINKING(SwirlyLightComponentDesc)
	FORCE_LINKING(CelestialBodyComponentDesc)
	FORCE_LINKING(PlaylistComponentDesc)
}


} // namespace demo


