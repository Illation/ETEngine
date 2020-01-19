#pragma once
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Components/AudioSourceComponent.h>

#include <Common/PlaylistComponent.h>


namespace demo {


//---------------------------
// PlaylistSystemView
//
struct PlaylistSystemView final : public fw::ComponentView
{
	PlaylistSystemView() : fw::ComponentView()
	{
		Declare(playlist);
		Declare(source);
	}

	WriteAccess<PlaylistComponent> playlist;
	WriteAccess<fw::AudioSourceComponent> source;
};

//---------------------------
// PlaylistSystem
//
// switch through tracks
//
class PlaylistSystem final : public fw::System<PlaylistSystem, PlaylistSystemView>
{
public:
	PlaylistSystem();

	void Process(fw::ComponentRange<PlaylistSystemView>& range) override;
};


} // namespace demo

