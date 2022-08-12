#include "stdafx.h"
#include "linkerHelper.h"

#include <EtPipeline/Core/linkerHelper.h>
#include <EtPipeline/RHI/linkerHelper.h>
#include <EtPipeline/GUI/linkerHelper.h>
#include <EtPipeline/Rendering/linkerHelper.h>
#include <EtPipeline/Framework/linkerHelper.h>

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
	pl::ForceLinking_RHI();
	pl::ForceLinking_GUI();
	pl::ForceLinking_Rendering();
	pl::ForceLinking_Framework();

	FORCE_LINKING(EditableTestAsset)
}


} // namespace demo
} // namespace et
