#pragma once
#include <EtFramework/Components/AudioListenerComponent.h>
#include <EtFramework/Components/TransformComponent.h>

#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>


namespace fw {


//---------------------------------
// AudioListenerSystemView
//
// Data required for the audio listener system to process
//
struct AudioListenerSystemView final : public ComponentView
{
	AudioListenerSystemView() : ComponentView()
	{
		Declare(listener);
		Declare(transf);
		Declare(_active);
	}

	WriteAccess<AudioListenerComponent> listener;
	ReadAccess<TransformComponent> transf;

	ReadAccess<ActiveAudioListenerComponent> _active;
};

//---------------------
// AudioListenerSystem
//
// Synchronizes the active audio listener with openAl
//
class AudioListenerSystem final : public fw::System<AudioListenerSystem, AudioListenerSystemView>
{
public:
	AudioListenerSystem();

	void Process(ComponentRange<AudioListenerSystemView>& range) const override;
};


} // namespace fw

