#pragma once
#include <EtFramework/SceneGraph/ComponentDescriptor.h>


namespace et {
namespace fw {


//---------------------------------
// AudioListenerComponent
//
// Descriptor for serialization and deserialization of sprite components
//
class AudioListenerComponent final : public SimpleComponentDescriptor
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT
	RTTR_ENABLE(SimpleComponentDescriptor) // for serialization
	DECLARE_FORCED_LINKING()

	friend class AudioListenerSystem;

	// construct destruct
	//--------------------
public:
	AudioListenerComponent(float const gain = 1.f) : m_Gain(gain) {}
	~AudioListenerComponent() {}

	// accessors
	//-----------
	float GetGain() const { return m_Gain; }

	// modifiers
	//-----------
	void SetGain(float const val) { m_Gain = val; }

	// Data
	///////

private:
	vec3 m_PrevPos;
	float m_Gain = 1.f;
};

//---------------------------------
// ActiveAudioListenerComponent
//
// Tag component that marks an entity as being the active audio listener - shouldn't switch particularly often
//
struct ActiveAudioListenerComponent final
{
	ECS_DECLARE_COMPONENT
};


} // namespace fw
} // namespace et
