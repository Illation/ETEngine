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
		Include<ActiveAudioListenerComponent>();
	}

	WriteAccess<AudioListenerComponent> listener;
	ReadAccess<TransformComponent> transf;
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

	void Process(ComponentRange<AudioListenerSystemView>& range) override;
};


} // namespace fw

