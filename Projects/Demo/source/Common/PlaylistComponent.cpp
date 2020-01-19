#include "stdafx.h"
#include "PlaylistComponent.h"

#include <rttr/registration>

#include <EtCore/Content/ResourceManager.h>

#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/AudioSourceComponent.h>
#include <EtFramework/Audio/AudioData.h>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<demo::PlaylistComponent>("playlist component");

	registration::class_<demo::PlaylistComponentDesc>("playlist comp desc")
		.constructor<demo::PlaylistComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("tracks", &demo::PlaylistComponentDesc::tracks);

	rttr::type::register_converter_func([](demo::PlaylistComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new demo::PlaylistComponentDesc(descriptor);
	});
}

DEFINE_FORCED_LINKING(demo::PlaylistComponentDesc) // force the linker to include this unit

// component registration
//------------------------

ECS_REGISTER_COMPONENT(demo::PlaylistComponent);


namespace demo {


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
		ret->tracks.push_back(ResourceManager::Instance()->GetAssetData<AudioData>(GetHash(track)));
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
	auto const findResult = std::find_if(comp.tracks.cbegin(), comp.tracks.cend(), [sourceTrack](AssetPtr<AudioData> const& data)
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



