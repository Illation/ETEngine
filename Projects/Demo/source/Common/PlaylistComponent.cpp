#include "stdafx.h"
#include "PlaylistComponent.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/AudioSourceComponent.h>
#include <EtFramework/Audio/AudioData.h>


namespace et {
namespace demo {


//====================
// Playlist Component 
//====================


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(PlaylistComponent, "playlist component")
		.property("tracks", &PlaylistComponent::tracks)
	END_REGISTER_CLASS_POLYMORPHIC(PlaylistComponent, fw::I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(PlaylistComponent) // force the linker to include this unit

ECS_REGISTER_COMPONENT(PlaylistComponent);


//-------------------------------------------
// PlaylistComponent::OnScenePostLoadRoot
//
// Determine the current track based on the active track in the source component
//
void PlaylistComponent::OnScenePostLoadRoot(fw::EcsController& ecs, fw::T_EntityId const id, void* const componentData)
{
	PlaylistComponent& comp = *static_cast<PlaylistComponent*>(componentData);

	// access source component
	ET_ASSERT(ecs.HasComponent<fw::AudioSourceComponent>(id));
	fw::AudioSourceComponent& source = ecs.GetComponent<fw::AudioSourceComponent>(id);

	// if the source component has no data set we are done
	core::HashString const sourceTrack = source.GetTrack();
	if (sourceTrack == 0u)
	{
		comp.currentTrack = std::numeric_limits<size_t>::max();
		return;
	}

	// otherwise find it in the playlist
	auto const findResult = std::find_if(comp.tracks.cbegin(), comp.tracks.cend(), [sourceTrack](AssetPtr<fw::AudioData> const& data)
		{
			return (data.GetId() == sourceTrack);
		});

	if (findResult != comp.tracks.cend())
	{
		comp.currentTrack = findResult - comp.tracks.cbegin();
	}
	else
	{
		ET_WARNING("playlist doesn't include track set on the source component!");
		comp.currentTrack = std::numeric_limits<size_t>::max();
	}
}


} // namespace demo
} // namespace et
