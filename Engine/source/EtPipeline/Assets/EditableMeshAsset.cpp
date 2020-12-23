#include "stdafx.h"
#include "EditableMeshAsset.h"


namespace et {
namespace pl {


//=====================
// Editable Mesh Asset
//=====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableMeshAsset, "editable mesh asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableMeshAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableMeshAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
