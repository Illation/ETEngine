#include "stdafx.h"

#include "AudioListenerComponent.h"

#include <AL/al.h>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Audio/OpenAL_ETM.h>


// deprecated
//------------


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


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::AudioListenerComponent>("audio listener component");

	registration::class_<fw::AudioListenerComponentDesc>("audio listener comp desc")
		.constructor<fw::AudioListenerComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("gain", &fw::AudioListenerComponentDesc::gain);

	rttr::type::register_converter_func([](fw::AudioListenerComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::AudioListenerComponentDesc(descriptor);
	});
}


// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::AudioListenerComponent);
ECS_REGISTER_COMPONENT(fw::ActiveAudioListenerComponent);


namespace fw {


//=====================================
// Audio Listener Component Descriptor
//=====================================


//--------------------------------------
// AudioListenerComponentDesc::MakeData
//
// Create a sprite component from a descriptor
//
AudioListenerComponent* AudioListenerComponentDesc::MakeData()
{
	return new AudioListenerComponent(gain);
}


} // namespace fw