#pragma once
#include <EtRendering/linkerHelper.h>

#include <EtGUI/Content/SpriteFont.h>


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

	FORCE_LINKING(FontAsset)
}


} // namespace gui
} // namespace et
