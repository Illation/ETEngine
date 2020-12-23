#include "stdafx.h"
#include "EditableMaterialAsset.h"


namespace et {
namespace pl {


//=========================
// Editable Material Asset
//=========================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableMaterialAsset, "editable material asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableMaterialAsset, EditorAssetBase);

	BEGIN_REGISTER_CLASS(EditableMaterialInstanceAsset, "editable material instance asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableMaterialInstanceAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableMaterialAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
