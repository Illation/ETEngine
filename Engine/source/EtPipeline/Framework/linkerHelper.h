#pragma once
#include <EtPipeline/Framework/EditableAudioAsset.h>
#include <EtPipeline/Framework/EditableSceneDescriptor.h>


namespace et {
namespace pl {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking_Framework()
{
	FORCE_LINKING(EditableAudioAsset)
	FORCE_LINKING(EditableSceneDescriptorAsset)
}


} // namespace pl
} // namespace et

