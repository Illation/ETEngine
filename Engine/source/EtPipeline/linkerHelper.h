#pragma once
#include <EtPipeline/Assets/EditableAssetStub.h>


namespace et {
namespace pl {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	FORCE_LINKING(EditableStubAsset)
}


} // namespace pl
} // namespace et

