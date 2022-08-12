#pragma once
#include <EtPipeline/GUI/EditableSdfFont.h>
#include <EtPipeline/GUI/EditableGuiDocument.h>


namespace et {
namespace pl {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking_GUI()
{
	FORCE_LINKING(EditableSdfFontAsset)
	FORCE_LINKING(EditableGuiDocumentAsset)
}


} // namespace pl
} // namespace et

