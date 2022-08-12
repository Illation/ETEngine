#pragma once
#include <EtPipeline/RHI/EditableShaderAsset.h>
#include <EtPipeline/RHI/EditableTextureAsset.h>


namespace et {
namespace pl {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking_RHI()
{
	FORCE_LINKING(EditableShaderAsset)
	FORCE_LINKING(EditableTextureAsset)
}


} // namespace pl
} // namespace et

