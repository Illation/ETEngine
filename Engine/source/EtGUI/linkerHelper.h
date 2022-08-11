#pragma once
#include <EtRHI/linkerHelper.h>

#include <EtGUI/Context/GuiDocument.h>
#include <EtGUI/Fonts/SdfFont.h>


namespace et {
namespace gui {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	rhi::ForceLinking();

	FORCE_LINKING(SdfFontAsset)
	FORCE_LINKING(GuiDocumentAsset)
}


} // namespace gui
} // namespace et
