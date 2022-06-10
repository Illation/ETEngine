#pragma once
#include <EtRendering/linkerHelper.h>

#include <EtGUI/Content/FreetypeFont.h>
#include <EtGUI/Content/GuiDocument.h>
#include <EtGUI/Content/SdfFont.h>
#include <EtGUI/Rendering/FontEffects.h>


namespace et {
namespace gui {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	render::ForceLinking();

	FORCE_LINKING(SdfFontAsset)
	FORCE_LINKING(FreetypeFontAsset)
	FORCE_LINKING(GuiDocumentAsset)
}


} // namespace gui
} // namespace et
