#include "stdafx.h"
#include "EditableTextureAsset.h"


namespace et {
namespace pl {


//========================
// Editable Texture Asset
//========================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableTextureAsset, "editable texture asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableTextureAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableTextureAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
