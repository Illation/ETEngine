#include "stdafx.h"
#include "EditableTestAsset.h"


namespace et {
namespace demo {


//=====================
// Editable Test Asset
//=====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableTestAsset, "editable test asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableTestAsset, pl::EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableTestAsset) // force the asset class to be linked as it is only used in reflection


} // namespace demo
} // namespace et
