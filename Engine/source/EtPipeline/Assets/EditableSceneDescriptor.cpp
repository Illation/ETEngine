#include "stdafx.h"
#include "EditableSceneDescriptor.h"


namespace et {
namespace pl {


//=================================
// Editable Scene Descriptor Asset
//=================================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableSceneDescriptorAsset, "editable scene descriptor asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableSceneDescriptorAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableSceneDescriptorAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
