#include "stdafx.h"
#include "AudioSourceComponent.h"

#include <rttr/registration>

#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Audio/OpenAL_ETM.h>


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
