#pragma once
#include <EtFramework/Audio/AudioData.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableAudioAsset
//
class EditableAudioAsset final : public EditorAsset<fw::AudioData>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<fw::AudioData>)
public:
	// Construct destruct
	//---------------------
	EditableAudioAsset() : EditorAsset<fw::AudioData>() {}
	virtual ~EditableAudioAsset() = default;
};


} // namespace pl
} // namespace et
