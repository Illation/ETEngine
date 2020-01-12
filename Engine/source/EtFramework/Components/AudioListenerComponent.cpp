#include "stdafx.h"
#include "AudioListenerComponent.h"

#include <rttr/registration>


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