#include <EtGUI/stdafx.h>
#include "EditableGuiDocument.h"


namespace et {
namespace pl {


//=============================
// Editable GUI Document Asset
//=============================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableGuiDocumentAsset, "editable gui document asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableGuiDocumentAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableGuiDocumentAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et

