#include "stdafx.h"
#include "EditableFontAsset.h"


namespace et {
namespace pl {


//=====================
// Editable Font Asset
//=====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableFontAsset, "editable font asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableFontAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableFontAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
