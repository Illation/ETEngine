#pragma once
#include <EtPipeline/Rendering/EditableEnvironmentMap.h>
#include <EtPipeline/Rendering/EditableMaterialAsset.h>
#include <EtPipeline/Rendering/EditableMeshAsset.h>


namespace et {
namespace pl {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking_Rendering()
{
	FORCE_LINKING(EditableEnvironmentMapAsset)
	FORCE_LINKING(EditableMaterialAsset)
	FORCE_LINKING(EditableMeshAsset)
}


} // namespace pl
} // namespace et

