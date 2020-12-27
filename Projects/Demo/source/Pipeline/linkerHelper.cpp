#include "stdafx.h"
#include "linkerHelper.h"

#include <EtPipeline/linkerHelper.h>
#include <Common/linkerHelper.h>

#include "EditableTestAsset.h"


namespace et {
namespace demo {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinkingPipeline()
{
	demo::ForceLinking(); // makes sure the linker doesn't ignore reflection only data
	pl::ForceLinking();

	FORCE_LINKING(EditableTestAsset)
}


} // namespace demo
} // namespace et
