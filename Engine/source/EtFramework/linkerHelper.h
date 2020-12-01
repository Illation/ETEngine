#pragma once

#include <EtRendering/linkerHelper.h>

#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/SceneGraph/SceneDescriptor.h>
#include <EtFramework/Physics/CollisionShape.h>
#include <EtFramework/Components/AudioListenerComponent.h>


namespace et {
namespace fw {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	render::ForceLinking();

	FORCE_LINKING(AudioAsset)
	FORCE_LINKING(SceneDescriptorAsset)
	FORCE_LINKING(BoxShape)
	FORCE_LINKING(AudioListenerComponent)
}


} // namespace fw
} // namespace et
