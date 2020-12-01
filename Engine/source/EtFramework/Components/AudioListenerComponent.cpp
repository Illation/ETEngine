#include "stdafx.h"
#include "AudioListenerComponent.h"

#include <EtCore/Reflection/Registration.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(AudioListenerComponent, "audio listener component")
		.property("gain", &AudioListenerComponent::GetGain, &AudioListenerComponent::SetGain)
	END_REGISTER_CLASS_POLYMORPHIC(AudioListenerComponent, I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(AudioListenerComponent) // force the linker to include this unit

ECS_REGISTER_COMPONENT(AudioListenerComponent);
ECS_REGISTER_COMPONENT(ActiveAudioListenerComponent);


} // namespace fw
} // namespace et