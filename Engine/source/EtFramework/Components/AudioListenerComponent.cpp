#include "stdafx.h"

#include "AudioListenerComponent.h"

#include <AL/al.h>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Audio/OpenAL_ETM.h>


void AudioListenerComponent::SetGain(float val)
{
	m_Gain = val;

	if (GetEntity()->GetScene()->GetAudioListener() != this) return;

	alListenerf(AL_GAIN, m_Gain);
	if (AudioManager::GetInstance()->TestALError("AudioListenerComponent set gain error"))return;
}

void AudioListenerComponent::Init()
{
	AbstractScene* const scene = GetEntity()->GetScene();
	if (scene->GetAudioListener() && scene->GetAudioListener() != this)
	{
		LOG("Scene already has an audio listener!", Warning);
	}

	scene->SetAudioListener(this);
}

void AudioListenerComponent::Deinit()
{
	AbstractScene* const scene = GetEntity()->GetScene();
	if (scene->GetAudioListener() == this)
	{
		scene->SetAudioListener(nullptr);
	}
}

void AudioListenerComponent::Update()
{
	vec3 pos = ALvec3(TRANSFORM->GetPosition());
	vec3 vel = pos - m_PrevPos;
	m_PrevPos = pos;

	if (GetEntity()->GetScene()->GetAudioListener() != this) return;

	alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
	if (AudioManager::GetInstance()->TestALError("AudioListenerComponent set position error"))return;

	alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
	if (AudioManager::GetInstance()->TestALError("AudioListenerComponent set velocity error"))return;

	vec3 fwd = ALvec3(TRANSFORM->GetForward());
	vec3 up = ALvec3(TRANSFORM->GetUp());
	ALfloat listenerOri[] = { fwd.x, fwd.y, fwd.z, up.x, up.y, up.z };

	alListenerfv(AL_ORIENTATION, listenerOri);
	if (AudioManager::GetInstance()->TestALError("AudioListenerComponent set orientation error"))return;
}