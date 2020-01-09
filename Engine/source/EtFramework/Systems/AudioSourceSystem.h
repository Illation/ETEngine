#pragma once
#include <EtFramework/Components/AudioSourceComponent.h>
#include <EtFramework/Components/TransformComponent.h>

#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/EcsController.h>


namespace fw {


//-------------------
// AudioSourceSystem
//
// Updates audio sources
//  - runs in two phases (translate -> state) 
//
class AudioSourceSystem final
{
public:
	// Init / Deinit
	//----------------
	static void OnComponentAdded(EcsController& controller, AudioSourceComponent& component, T_EntityId const entity);
	static void OnComponentRemoved(EcsController& controller, AudioSourceComponent& component, T_EntityId const entity);

	//---------------------------------
	// Translate
	//
	// Updates the position and orientation of non global sources
	//

	struct TranslateView final : public ComponentView
	{
		TranslateView() : ComponentView()
		{
			Declare(source3D);
			Declare(source);
			Declare(transf);
		}

		WriteAccess<AudioSource3DComponent> source3D;
		ReadAccess<AudioSourceComponent> source;
		ReadAccess<TransformComponent> transf;
	};

	class Translate final : public fw::System<Translate, TranslateView>
	{
	public:
		Translate();

		void Process(ComponentRange<TranslateView>& range) const override;
	};


	//---------------------------------
	// State
	//
	// Update the playback state of audio sources 
	//

	struct StateView final : public ComponentView
	{
		StateView() : ComponentView()
		{
			Declare(source);
		}

		WriteAccess<AudioSourceComponent> source;
	};

	class State final : public fw::System<State, StateView>
	{
	public:
		State() = default;

		void Process(ComponentRange<StateView>& range) const override;
	};
};


} // namespace fw

