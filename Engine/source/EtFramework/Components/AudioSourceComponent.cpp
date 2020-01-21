#include "stdafx.h"
#include "AudioSourceComponent.h"

#include <EtCore/Reflection/Registration.h>

#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Audio/OpenAL_ETM.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<AudioSourceComponent>("audio source component");

	rttr::registration::class_<AudioSource3DParams>("audio source 3D params")
		.property("reference distance", &AudioSource3DParams::referenceDistance)
		.property("rolloff factor", &AudioSource3DParams::rolloffFactor)
		.property("max distance", &AudioSource3DParams::maxDistance)
		.property("inner cone angle", &AudioSource3DParams::innerConeAngle)
		.property("outer cone angle", &AudioSource3DParams::outerConeAngle)
		.property("outer cone gain", &AudioSource3DParams::outerConeGain)
		.property("is directional", &AudioSource3DParams::isDirectional);

	rttr::registration::class_<fw::AudioSourceComponent>("audio source 3D component");
	
	BEGIN_REGISTER_POLYMORPHIC_CLASS(AudioSourceCompDesc, "audio source comp desc")
		.property("audio asset", &AudioSourceCompDesc::audioAsset)
		.property("play on init", &AudioSourceCompDesc::playOnInit)
		.property("loop", &AudioSourceCompDesc::loop)
		.property("gain", &AudioSourceCompDesc::gain)
		.property("min gain", &AudioSourceCompDesc::minGain)
		.property("max gain", &AudioSourceCompDesc::maxGain)
		.property("pitch", &AudioSourceCompDesc::pitch)
		.property("params", &AudioSourceCompDesc::params)
	END_REGISTER_POLYMORPHIC_CLASS(AudioSourceCompDesc, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(AudioSourceComponent);
ECS_REGISTER_COMPONENT(AudioSource3DComponent);


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
} // namespace et
