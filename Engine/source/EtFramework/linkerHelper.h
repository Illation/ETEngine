#pragma once

#include <EtRendering/linkerHelper.h>

#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/SceneGraph/SceneDescriptor.h>
#include <EtFramework/Physics/CollisionShape.h>


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	render::ForceLinking();

	FORCE_LINKING(AudioAsset)
	fw::FORCE_LINKING(SceneDescriptorAsset)
	fw::FORCE_LINKING(BoxShape)
}


