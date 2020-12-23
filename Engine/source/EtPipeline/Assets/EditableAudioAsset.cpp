#include "stdafx.h"
#include "EditableAudioAsset.h"


namespace et {
namespace pl {


//======================
// Editable Audio Asset
//======================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableAudioAsset, "editable audio asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableAudioAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableAudioAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
