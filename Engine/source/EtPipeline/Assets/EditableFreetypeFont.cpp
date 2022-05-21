#include "stdafx.h"
#include "EditableFreetypeFont.h"


namespace et {
namespace pl {


//==============================
// Editable Freetypr Font Asset
//==============================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableFreetypeFontAsset, "editable freetype font asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableFreetypeFontAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableFreetypeFontAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
