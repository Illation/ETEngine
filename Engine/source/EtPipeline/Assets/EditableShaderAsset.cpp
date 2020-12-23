#include "stdafx.h"
#include "EditableShaderAsset.h"


namespace et {
namespace pl {


//=======================
// Editable Shader Asset
//=======================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableShaderAsset, "editable shader asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableShaderAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableShaderAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
