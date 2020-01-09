#include "stdafx.h"
#include "AudioSourceComponent.h"

#include <rttr/registration>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Audio/OpenAL_ETM.h>


// deprecated
//------------


void AudioSourceComponent::Init()
{
	alGenSources((ALuint)1, &m_Source);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent generate source error");

	m_IsInitialized = true;

	//Set those values incase they where defined before the source was generated
	SetGain(m_Gain);
	SetMinGain(m_MinGain);
	SetMaxGain(m_MaxGain);
	SetPitch(m_Pitch);
	SetLooping(m_IsLooping);
	SetInnerConeAngle(m_InnerConeAngle);
	SetOuterConeAngle(m_OuterConeAngle);
	SetOuterConeGain(m_OuterConeGain);
	SetReferenceDistance(m_ReferenceDistance);
	SetRolloffFactor(m_RolloffFactor);
	SetMaxDistance(m_MaxDistance);

	if (m_pAudioData != nullptr)
	{
		alSourcei(m_Source, AL_BUFFER, m_pAudioData->GetHandle());
		AudioManager::GetInstance()->TestALError("AudioSourceComponent bind audioData buffer error");

		if (m_IsPlaying && !m_IsPaused)
		{
			alSourcePlay(m_Source);
		}
	}
}

void AudioSourceComponent::Deinit()
{
	alDeleteSources(1, &m_Source);
}

void AudioSourceComponent::SetGain(float val)
{
	m_Gain = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_GAIN, m_Gain);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set gain error");
}

void AudioSourceComponent::SetMinGain(float val)
{
	m_MinGain = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_MIN_GAIN, m_MinGain);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set min gain error");
}

void AudioSourceComponent::SetMaxGain(float val)
{
	m_MaxGain = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_MAX_GAIN, m_MaxGain);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set max gain error");
}

void AudioSourceComponent::SetPitch(float val)
{
	m_Pitch = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_PITCH, m_Pitch);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set pitch error");
}

void AudioSourceComponent::SetLooping(bool val)
{
	m_IsLooping = val;

	if (!m_IsInitialized)return;

	alSourcei(m_Source, AL_LOOPING, m_IsLooping ? AL_TRUE : AL_FALSE);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set IsLooping error");
}

void AudioSourceComponent::SetAudioData(AssetPtr<AudioData> const& data)
{
	if (m_IsInitialized && (m_pAudioData != nullptr))
	{
		AudioManager::GetInstance()->TestALError("AudioSourceComponent::SetAudioData > pre");
		if (m_IsPlaying)
		{
			alSourceStop(m_Source);
		}
		alSourcei(m_Source, AL_BUFFER, 0);
		AudioManager::GetInstance()->TestALError("AudioSourceComponent::SetAudioData > unset");
	}

	m_pAudioData = data;
	if (m_IsInitialized)
	{
		alSourcei(m_Source, AL_BUFFER, m_pAudioData->GetHandle());
		AudioManager::GetInstance()->TestALError("AudioSourceComponent::SetAudioData > set");

		if (m_IsPlaying && !m_IsPaused)
		{
			alSourcePlay(m_Source);
		}
	}
}

void AudioSourceComponent::Play()
{
	m_IsPlaying = true;
	m_IsPaused = false;
	if ((m_pAudioData != nullptr) && m_IsInitialized)
	{
		alSourcePlay(m_Source);
	}
}

void AudioSourceComponent::Pause()
{
	if (m_IsPlaying)
	{
		m_IsPlaying = false;
		m_IsPaused = true;

		if(m_IsInitialized) alSourcePause(m_Source);
	}
}

void AudioSourceComponent::Stop()
{
	m_IsPlaying = false;
	m_IsPaused = false;
	if(m_IsInitialized) alSourceStop(m_Source);
}

void AudioSourceComponent::SetIsDirectional(bool val)
{
	m_IsDirectional = val;
	if ((!val) && m_IsInitialized)
	{
		alSource3i(m_Source, AL_DIRECTION, 0, 0, 0);
		if (AudioManager::GetInstance()->TestALError("AudioSourceComponent set direction error"))return;
	}
}

void AudioSourceComponent::SetInnerConeAngle(float val)
{
	m_InnerConeAngle = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_CONE_INNER_ANGLE, m_InnerConeAngle);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set inner cone angle error");
}
void AudioSourceComponent::SetOuterConeAngle(float val)
{
	m_OuterConeAngle = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_CONE_OUTER_ANGLE, m_OuterConeAngle);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set outer cone angle error");
}
void AudioSourceComponent::SetOuterConeGain(float val)
{
	m_OuterConeGain = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_CONE_OUTER_GAIN, m_OuterConeGain);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set outer cone gain error");
}

void AudioSourceComponent::SetReferenceDistance(float val)
{
	m_ReferenceDistance = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_REFERENCE_DISTANCE, m_ReferenceDistance);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set reference distance error");
}
void AudioSourceComponent::SetRolloffFactor(float val)
{
	m_RolloffFactor = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_ROLLOFF_FACTOR, m_RolloffFactor);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set rolloff factor error");
}
void AudioSourceComponent::SetMaxDistance(float val)
{
	m_MaxDistance = val;

	if (!m_IsInitialized)return;

	alSourcef(m_Source, AL_MAX_DISTANCE, m_MaxDistance);
	AudioManager::GetInstance()->TestALError("AudioSourceComponent set max distance error");
}

void AudioSourceComponent::Update()
{
	vec3 pos = ALvec3(TRANSFORM->GetPosition());
	alSource3f(m_Source, AL_POSITION, pos.x, pos.y, pos.z);
	if (AudioManager::GetInstance()->TestALError("AudioSourceComponent set position error"))return;

	vec3 vel = pos - m_PrevPos;
	m_PrevPos = pos;
	alSource3f(m_Source, AL_VELOCITY, vel.x, vel.y, vel.z);
	if (AudioManager::GetInstance()->TestALError("AudioSourceComponent set velocity error"))return;

	if (m_IsDirectional)
	{
		vec3 fwd = ALvec3(TRANSFORM->GetForward());
		alSource3f(m_Source, AL_DIRECTION, fwd.x, fwd.y, fwd.z);
		if (AudioManager::GetInstance()->TestALError("AudioSourceComponent set direction error"))return;
	}

	ALint sourceState;
	alGetSourcei(m_Source, AL_SOURCE_STATE, &sourceState);
	if (AudioManager::GetInstance()->TestALError("AudioSourceComponent get source state error"))return;
	switch (sourceState)
	{
	case AL_PLAYING:
		m_IsPlaying = true;
		m_IsPaused = true;
	case AL_PAUSED:
		m_IsPlaying = false;
		m_IsPaused = true;
	default:
		if ((m_pAudioData != nullptr) && m_IsInitialized)
		{
			m_IsPlaying = false;
			m_IsPaused = false;
		}
	}
}


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::AudioSourceComponent>("audio source component");

	registration::class_<fw::AudioSource3DParams>("audio source 3D params")
		.property("reference distance", &fw::AudioSource3DParams::referenceDistance)
		.property("rolloff factor", &fw::AudioSource3DParams::rolloffFactor)
		.property("max distance", &fw::AudioSource3DParams::maxDistance)
		.property("inner cone angle", &fw::AudioSource3DParams::innerConeAngle)
		.property("outer cone angle", &fw::AudioSource3DParams::outerConeAngle)
		.property("outer cone gain", &fw::AudioSource3DParams::outerConeGain)
		.property("is directional", &fw::AudioSource3DParams::isDirectional);
	registration::class_<fw::AudioSourceComponent>("audio source 3D component");

	registration::class_<fw::AudioSourceCompDesc>("audio source comp desc")
		.constructor<fw::AudioSourceCompDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("audio asset", &fw::AudioSourceCompDesc::audioAsset)
		.property("play on init", &fw::AudioSourceCompDesc::playOnInit)
		.property("loop", &fw::AudioSourceCompDesc::loop)
		.property("gain", &fw::AudioSourceCompDesc::gain)
		.property("max gain", &fw::AudioSourceCompDesc::minGain)
		.property("min gain", &fw::AudioSourceCompDesc::maxGain)
		.property("pitch", &fw::AudioSourceCompDesc::pitch)
		.property("params", &fw::AudioSourceCompDesc::params);

	rttr::type::register_converter_func([](fw::AudioSourceCompDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::AudioSourceCompDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::AudioSourceComponent);
ECS_REGISTER_COMPONENT(fw::AudioSource3DComponent);


namespace fw {


//========================
// Audio Source Component 
//========================


//---------------------------------
// AudioSourceComponent::c-tor
//
AudioSourceComponent::AudioSourceComponent(float const gain, float const minGain, float const maxGain, float const pitch)
	: m_Gain(gain)
	, m_MinGain(minGain)
	, m_MaxGain(maxGain)
	, m_Pitch(pitch)
{ }


//===================================
// Audio Source Component Descriptor
//===================================


//------------------------------
// AudioSourceCompDesc::MakeData
//
// Create an audio source component from a descriptor
//
AudioSourceComponent* AudioSourceCompDesc::MakeData()
{
	AudioSourceComponent* const comp = new AudioSourceComponent(gain, minGain, maxGain, pitch);

	comp->SetAudioData(GetHash(audioAsset));
	comp->SetLooping(loop);

	if (playOnInit)
	{
		comp->SetState(AudioSourceComponent::E_PlaybackState::Playing);
	}

	return comp;
}

//--------------------------------------
// AudioSourceCompDesc::OnScenePostLoad
//
// Create an audio source component from a descriptor
//
void AudioSourceCompDesc::OnScenePostLoad(EcsController& ecs, T_EntityId const id, AudioSourceComponent& comp)
{
	UNUSED(comp);

	if (ecs.HasComponent<AudioSource3DComponent>(id))
	{
		ecs.GetComponent<AudioSource3DComponent>(id).SetParams(params);
	}
}


} // namespace fw
