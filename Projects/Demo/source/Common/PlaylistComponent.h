#pragma once
#include <EtCore/Util/LinkerUtils.h>
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


namespace et { namespace fw {
	class AudioData;
} }


namespace et {
namespace demo {


//---------------------------------
// PlaylistComponent
//
// List of audio tracks that can be rotated through
//
struct PlaylistComponent final : public fw::SimpleComponentDescriptor
{
	ECS_DECLARE_COMPONENT
	RTTR_ENABLE(fw::SimpleComponentDescriptor)
	DECLARE_FORCED_LINKING()
public:

	bool CallScenePostLoad() const override { return true; }
	void OnScenePostLoadRoot(fw::EcsController& ecs, fw::T_EntityId const id, void* const componentData) override;

	size_t currentTrack = 0u;

	// by holding pointers instead of IDs, we ensure the audio data is loaded and doesn't have to reloaded when we switch tracks
	//  - we could be smart about it and only hold pointers of the current, next and previous track in order to keep memory requirements down
	std::vector<AssetPtr<fw::AudioData>> tracks; 
};


} // namespace demo
} // namespace et
