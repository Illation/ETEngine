#include "stdafx.h"
#include "PlaylistComponent.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/AudioSourceComponent.h>
#include <EtFramework/Audio/AudioData.h>


namespace et {
namespace demo {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<PlaylistComponent>("playlist component");

	BEGIN_REGISTER_POLYMORPHIC_CLASS(PlaylistComponentDesc, "playlist comp desc")
		.property("tracks", &PlaylistComponentDesc::tracks)
	END_REGISTER_POLYMORPHIC_CLASS(PlaylistComponentDesc, fw::I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(PlaylistComponentDesc) // force the linker to include this unit

ECS_REGISTER_COMPONENT(PlaylistComponent);


//===============================
// Playlist Component Descriptor
//===============================


//---------------------------------------
// PlaylistComponentDesc::MakeData
//
PlaylistComponent* PlaylistComponentDesc::MakeData()
{
	PlaylistComponent* const ret = new PlaylistComponent();

	for (std::string const& track : tracks)
	{
		ret->tracks.push_back(ResourceManager::Instance()->GetAssetData<fw::AudioData>(GetHash(track)));
	}

	return ret;
}

//-------------------------------------------
// PlaylistComponentDesc::OnScenePostLoad
//
// Determine the current track based on the active track in the source component
//
void PlaylistComponentDesc::OnScenePostLoad(fw::EcsController& ecs, fw::T_EntityId const id, PlaylistComponent& comp)
{
	// access source component
	ET_ASSERT(ecs.HasComponent<fw::AudioSourceComponent>(id));
	fw::AudioSourceComponent& source = ecs.GetComponent<fw::AudioSourceComponent>(id);

	// if the source component has no data set we are done
	T_Hash const sourceTrack = source.GetTrack();
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
		ET_ASSERT(false, "playlist doesn't include track set on the source component!");
		comp.currentTrack = std::numeric_limits<size_t>::max();
	}
}


} // namespace demo
} // namespace et
