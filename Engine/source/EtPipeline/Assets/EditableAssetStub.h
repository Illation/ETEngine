#pragma once
#include <EtCore/Content/AssetStub.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableStubAsset
//
class EditableStubAsset final : public EditorAsset<core::StubData>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<core::StubData>)
public:
	// Construct destruct
	//---------------------
	EditableStubAsset() : EditorAsset<core::StubData>() {}
	virtual ~EditableStubAsset() = default;
};


} // namespace pl
} // namespace et
