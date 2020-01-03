#pragma once

#include <EtRendering/linkerHelper.h>

#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/SceneGraph/SceneDescriptor.h>


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	render::ForceLinking();

	FORCE_LINKING(AudioAsset)
	framework::FORCE_LINKING(SceneDescriptorAsset)
}


