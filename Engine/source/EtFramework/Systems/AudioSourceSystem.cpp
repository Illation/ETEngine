#include "stdafx.h"
#include "AudioSourceSystem.h"

#include "TransformSystem.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Audio/OpenAL_ETM.h>


namespace et {
namespace fw {


//=====================
// Audio Source System 
//=====================


//-----------------------------------
// AudioSourceSystem::OnComponentAdded
//
// Register transform components in the render scene when they are added to the ECS
//
void AudioSourceSystem::OnComponentAdded(EcsController& controller, AudioSourceComponent& component, T_EntityId const entity)
{
	// generate the source handle
	alGenSources((ALuint)1, &component.m_Source);
	ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL gen sources error"));

	// are we 3D ?
	if (controller.HasComponent<TransformComponent>(entity))
	{
		controller.AddComponents(entity, AudioSource3DComponent());
	}
	else // make our component play audio globally
	{
		alSourcei(component.m_Source, AL_SOURCE_RELATIVE, AL_TRUE);
		ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source relative error"));

		// set the source dist to be the ref dist in front of the listener, so it will always be at default volume
		alSource3f(component.m_Source, AL_POSITION, 0.f, 0.f, -1.f);
		ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source position error"));
	}

	// set source params
	alSourcef(component.m_Source, AL_GAIN, component.m_Gain);
	ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source gain error"));

	alSourcef(component.m_Source, AL_MIN_GAIN, component.m_MinGain);
	ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source min gain error"));

	alSourcef(component.m_Source, AL_MAX_GAIN, component.m_MaxGain);
	ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source max gain error"));

	alSourcef(component.m_Source, AL_PITCH, component.m_Pitch);
	ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source pitch error"));

	alSourcei(component.m_Source, AL_LOOPING, (component.m_IsLooping ? AL_TRUE : AL_FALSE));
	ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source looping error"));

	// set the audio track
	//---------------------
	if (component.m_NextTrack != 0u)
	{
		component.m_AudioData = ResourceManager::Instance()->GetAssetData<AudioData>(component.m_NextTrack);
		alSourcei(component.m_Source, AL_BUFFER, component.m_AudioData->GetHandle());
		ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source buffer error"));
	}
}

//-------------------------------------
// AudioSourceSystem::OnComponentRemoved
//
// Remove respectively
//
void AudioSourceSystem::OnComponentRemoved(EcsController& controller, AudioSourceComponent& component, T_EntityId const entity)
{
	UNUSED(controller);
	UNUSED(entity);

	alDeleteSources(1, &component.m_Source);
	ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL delete sources error"));
}

//--------------------------------------
// AudioSourceSystem::Translate::c-tor
//
// dependency setup
//
AudioSourceSystem::Translate::Translate()
{
	DeclareDependencies<TransformSystem::Compute>(); // the rigid body system may update transformations

	// run before the transform components reset so we only update position data for audio sources that moved
	DeclareDependents<TransformSystem::Reset>(); 

	DeclareDependents<AudioSourceSystem::State>();
}

//---------------------------------------
// AudioSourceSystem::Translate::Process
//
// Update 3D source data within openAL
//
void AudioSourceSystem::Translate::Process(ComponentRange<AudioSourceSystem::TranslateView>& range) 
{
	for (TranslateView& view : range)
	{
		// 3D source params, if dirty
		if (view.source3D->m_UpdateParams)
		{
			alSourcef(view.source->m_Source, AL_REFERENCE_DISTANCE, view.source3D->m_Params.referenceDistance);
			alSourcef(view.source->m_Source, AL_ROLLOFF_FACTOR, view.source3D->m_Params.rolloffFactor);
			alSourcef(view.source->m_Source, AL_MAX_DISTANCE, view.source3D->m_Params.maxDistance);
			alSourcef(view.source->m_Source, AL_CONE_INNER_ANGLE, view.source3D->m_Params.innerConeAngle);
			alSourcef(view.source->m_Source, AL_CONE_OUTER_ANGLE, view.source3D->m_Params.outerConeAngle);
			alSourcef(view.source->m_Source, AL_CONE_OUTER_GAIN, view.source3D->m_Params.outerConeGain);

			if (!view.source3D->m_Params.isDirectional)
			{
				alSource3f(view.source->m_Source, AL_DIRECTION, 0.f, 0.f, 0.f);
			}
			else
			{
				vec3 const fwd = ALvec3(view.transf->GetForward());
				alSource3f(view.source->m_Source, AL_DIRECTION, fwd.x, fwd.y, fwd.z);
			}

			ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL 3D source params error"));

			view.source3D->m_UpdateParams = false;
		}

		// 3D source position, rotation, direction
		if (view.transf->HasTransformChanged())
		{
			vec3 const pos = ALvec3(view.transf->GetWorldPosition());
			alSource3f(view.source->m_Source, AL_POSITION, pos.x, pos.y, pos.z);

			vec3 const vel = pos - view.source3D->m_PrevPos;
			view.source3D->m_PrevPos = pos;
			alSource3f(view.source->m_Source, AL_VELOCITY, vel.x, vel.y, vel.z);

			if (view.source3D->m_Params.isDirectional)
			{
				vec3 const fwd = ALvec3(view.transf->GetForward());
				alSource3f(view.source->m_Source, AL_DIRECTION, fwd.x, fwd.y, fwd.z);
			}
		}
		else
		{
			alSource3f(view.source->m_Source, AL_VELOCITY, 0.f, 0.f, 0.f);
		}

		ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL 3D source transform error"));
	}
}

//------------------------------------
// AudioSourceSystem::State::Process
//
// Update non location dependent data for all audio sources
//
void AudioSourceSystem::State::Process(ComponentRange<AudioSourceSystem::StateView>& range) 
{
	for (StateView& view : range)
	{
		// audio track changed
		if (view.source->m_NextTrack != view.source->m_AudioData.GetId())
		{
			alSourceStop(view.source->m_Source);
			alSourcei(view.source->m_Source, AL_BUFFER, AL_NONE);

			if (view.source->m_NextTrack == 0u)
			{
				view.source->m_AudioData = nullptr;
			}
			else
			{
				view.source->m_AudioData = ResourceManager::Instance()->GetAssetData<AudioData>(view.source->m_NextTrack);
				alSourcei(view.source->m_Source, AL_BUFFER, view.source->m_AudioData->GetHandle());

				if (view.source->m_State == AudioSourceComponent::E_PlaybackState::Playing)
				{
					alSourcePlay(view.source->m_Source);
				}
				else if (view.source->m_State == AudioSourceComponent::E_PlaybackState::Paused)
				{
					view.source->m_State = AudioSourceComponent::E_PlaybackState::Stopped;
					view.source->m_PrevState = AudioSourceComponent::E_PlaybackState::Stopped;
				}
			}

			ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source buffer change error"));
		}

		// looping changed
		ALint loopingState;
		alGetSourcei(view.source->m_Source, AL_LOOPING, &loopingState);
		if ((loopingState == AL_TRUE) && !view.source->m_IsLooping)
		{
			alSourcei(view.source->m_Source, AL_LOOPING, AL_FALSE);
		}
		else if ((loopingState == AL_FALSE) && view.source->m_IsLooping)
		{
			alSourcei(view.source->m_Source, AL_LOOPING, AL_TRUE);
		}

		ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source looping error"));

		// state changed by external system
		if (view.source->m_State != view.source->m_PrevState) 
		{
			switch (view.source->m_State)
			{
			case AudioSourceComponent::E_PlaybackState::Playing:
				alSourcePlay(view.source->m_Source);
				break;

			case AudioSourceComponent::E_PlaybackState::Paused:
				alSourcePause(view.source->m_Source);
				break;

			case AudioSourceComponent::E_PlaybackState::Stopped:
				alSourceStop(view.source->m_Source);
				break;

			default:
				ET_ASSERT(false, "unhandled source state!");
				break;
			}

			ET_ASSERT(!AudioManager::GetInstance()->TestALError("AL source state transition error"));
			view.source->m_PrevState = view.source->m_State;
		}

		// sync state with what openAL is actually doing
		ALint sourceState;
		alGetSourcei(view.source->m_Source, AL_SOURCE_STATE, &sourceState);
		ET_ASSERT(!AudioManager::GetInstance()->TestALError("get AL source state error"));

		switch (sourceState)
		{
		case AL_PLAYING:
			view.source->m_State = AudioSourceComponent::E_PlaybackState::Playing;
			view.source->m_PrevState = AudioSourceComponent::E_PlaybackState::Playing;
			break;
		case AL_PAUSED:
			view.source->m_State = AudioSourceComponent::E_PlaybackState::Paused;
			view.source->m_PrevState = AudioSourceComponent::E_PlaybackState::Paused;
			break;
		default:
			view.source->m_State = AudioSourceComponent::E_PlaybackState::Stopped;
			view.source->m_PrevState = AudioSourceComponent::E_PlaybackState::Stopped;
			break;
		}
	}
}


} // namespace fw
} // namespace et
