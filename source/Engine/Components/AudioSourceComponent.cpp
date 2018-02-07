#include "stdafx.hpp"
#include "AudioSourceComponent.h"
#include "Entity.hpp"

#include "Audio/AudioData.h"

#include "Audio/AudioManager.h"
#include "Audio/OpenAL_ETM.h"

AudioSourceComponent::~AudioSourceComponent()
{
	alDeleteSources(1, &m_Source);
}

void AudioSourceComponent::Initialize()
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

	if (m_pAudioData)
	{
		alSourcei(m_Source, AL_BUFFER, m_pAudioData->GetHandle());
		AudioManager::GetInstance()->TestALError("AudioSourceComponent bind audioData buffer error");

		if (m_IsPlaying && !m_IsPaused)
		{
			alSourcePlay(m_Source);
		}
	}
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

void AudioSourceComponent::SetAudioData(AudioData* val)
{
	m_pAudioData = val;
	if (m_IsInitialized)
	{
		alSourcei(m_Source, AL_BUFFER, m_pAudioData->GetHandle());
		AudioManager::GetInstance()->TestALError("AudioSourceComponent bind audioData buffer error");

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
	if (m_pAudioData && m_IsInitialized)
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
		if (m_pAudioData && m_IsInitialized)
		{
			m_IsPlaying = false;
			m_IsPaused = false;
		}
	}
}