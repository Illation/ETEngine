#pragma once
#include <EtCore/Helper/LinkerUtils.h>
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


class AudioData;


namespace demo {


//---------------------------------
// PlaylistComponent
//
// List of audio tracks that can be rotated through
//
struct PlaylistComponent final
{
	ECS_DECLARE_COMPONENT
public:

	size_t currentTrack = 0u;

	// by holding pointers instead of IDs, we ensure the audio data is loaded and doesn't have to reloaded when we switch tracks
	//  - we could be smart about it and only hold pointers of the current, next and previous track in order to keep memory requirements down
	std::vector<AssetPtr<AudioData>> tracks; 
};


//---------------------------------
// PlaylistComponentDesc
//
class PlaylistComponentDesc final : public fw::ComponentDescriptor<PlaylistComponent, true>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<PlaylistComponent, true>)
	DECLARE_FORCED_LINKING()

	// construct destruct
	//--------------------
public:
	PlaylistComponentDesc() : ComponentDescriptor<PlaylistComponent, true>() {}

	// ComponentDescriptor interface
	//-------------------------------
	PlaylistComponent* MakeData() override;
	void OnScenePostLoad(fw::EcsController& ecs, fw::T_EntityId const id, PlaylistComponent& comp);

	std::vector<std::string> tracks;
};


} // namespace demo

