#include "stdafx.h"
#include "EditableEnvironmentMap.h"


namespace et {
namespace pl {


//================================
// Editable Environment Map Asset
//================================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableEnvironmentMapAsset, "editable environment map asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableEnvironmentMapAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableEnvironmentMapAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
