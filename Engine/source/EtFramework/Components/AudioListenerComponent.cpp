#include "stdafx.h"
#include "AudioListenerComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<AudioListenerComponent>("audio listener component");

	BEGIN_REGISTER_CLASS(AudioListenerComponentDesc, "audio listener comp desc")
		.property("gain", &AudioListenerComponentDesc::gain)
	END_REGISTER_CLASS_POLYMORPHIC(AudioListenerComponentDesc, I_ComponentDescriptor);
}

ECS_REGISTER_COMPONENT(AudioListenerComponent);
ECS_REGISTER_COMPONENT(ActiveAudioListenerComponent);


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
} // namespace et