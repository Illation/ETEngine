#pragma once
#include <EtPipeline/Content/EditorAsset.h>

#include <Common/CustomTestAsset.h>


namespace et {
namespace demo {
	

//---------------------------------
// EditableTestAsset
//
class EditableTestAsset final : public pl::EditorAsset<TestData>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(pl::EditorAsset<TestData>)
public:
	// Construct destruct
	//---------------------
	EditableTestAsset() : pl::EditorAsset<TestData>() {}
	virtual ~EditableTestAsset() = default;
};


} // namespace demo
} // namespace et
