#pragma once
#include <EtFramework/SceneGraph/SceneDescriptor.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableSceneDescriptorAsset
//
class EditableSceneDescriptorAsset final : public EditorAsset<fw::SceneDescriptor>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<fw::SceneDescriptor>)
public:
	// Construct destruct
	//---------------------
	EditableSceneDescriptorAsset() : EditorAsset<fw::SceneDescriptor>() {}
	virtual ~EditableSceneDescriptorAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	bool GenerateInternal(BuildConfiguration const& buildConfig) override;

	bool GenerateRequiresLoadData() const override { return true; }
};


} // namespace pl
} // namespace et
