#include <EtCore/stdafx.h>
#include "EditableAssetStub.h"


namespace et {
namespace pl {


//=====================
// Editable Stub Asset
//=====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableStubAsset, "editable stub asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableStubAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableStubAsset) // force the asset class to be linked as it is only used in reflection


} // namespace pl
} // namespace et
