#pragma once

#include <EtGUI/linkerHelper.h>

#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/SceneGraph/SceneDescriptor.h>
#include <EtFramework/Physics/CollisionShape.h>
#include <EtFramework/Components/AudioListenerComponent.h>
#include <EtFramework/Components/AtmosphereComponent.h>
#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/Components/LightComponent.h>
#include <EtFramework/Components/ModelComponent.h>
#include <EtFramework/Components/PlanetComponent.h>
#include <EtFramework/Components/RigidBodyComponent.h>
#include <EtFramework/Components/GuiCanvasComponent.h>


namespace et {
namespace fw {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	gui::ForceLinking();

	FORCE_LINKING(AudioAsset)
	FORCE_LINKING(SceneDescriptorAsset)
	FORCE_LINKING(BoxShape)
	FORCE_LINKING(AudioListenerComponent)
	FORCE_LINKING(AtmosphereComponentDesc)
	FORCE_LINKING(CameraComponent)
	FORCE_LINKING(LightComponentLinkEnforcer)
	FORCE_LINKING(ModelComponent)
	FORCE_LINKING(PlanetCameraLinkComponentDesc)
	FORCE_LINKING(RigidBodyComponentDesc)
	FORCE_LINKING(GuiCanvasComponentLinkEnforcer)
}


} // namespace fw
} // namespace et
