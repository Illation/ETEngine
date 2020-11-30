#pragma once
#include <AL/al.h>

#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/Audio/AudioData.h>
#include <EtFramework/SceneGraph/ComponentDescriptor.h>
#include <EtFramework/ECS/EcsController.h>


namespace et {
namespace fw {


//-----------------------
// AudioSourceComponent
//
// Component that plays sound effects at the location of the entity
//
class AudioSourceComponent final
{
	// definitions
	//---------------
	ECS_DECLARE_COMPONENT

	friend class AudioSourceSystem;

public:
	enum class E_PlaybackState : uint8
	{
		Stopped,
		Paused,
		Playing
	};

	// construct destruct
	//--------------------
	AudioSourceComponent(float const gain, float const minGain, float const maxGain, float const pitch);
	~AudioSourceComponent() = default;

	// modifiers
	//-----------
	void SetAudioData(AssetPtr<AudioData> const data) { m_NextTrack = data; }

	void SetState(E_PlaybackState const state) { m_State = state; }
	void SetLooping(bool const val) { m_IsLooping = val; }

	// accessors
	//-----------
	E_PlaybackState GetState() const { return m_PrevState; }
	bool IsLooping() const { return m_IsLooping; }

	core::HashString GetTrack() const { return m_NextTrack.GetId(); }

	// Data
	///////

private:
	// handle
	ALuint m_Source = 0u;

	// sfx
	AssetPtr<AudioData> m_AudioData;
	AssetPtr<AudioData> m_NextTrack;

	// sound modifiers (gain = volume / amplitude)
	float m_Gain = 1.f;
	float m_MinGain = 0.f;
	float m_MaxGain = 1.f;
	float m_Pitch = 1.f;

	// state
	E_PlaybackState m_State = E_PlaybackState::Stopped;
	E_PlaybackState m_PrevState = E_PlaybackState::Stopped;
	bool m_IsLooping = false;
};

//---------------------------------
// AudioSource3DParams
//
// How sound is modified relative to its 3D world location
//
struct AudioSource3DParams
{
	//Distance
	float referenceDistance = 1.f;
	float rolloffFactor = 1.f;
	float maxDistance = std::numeric_limits<float>::max();

	//Direction
	float innerConeAngle = 360.f;
	float outerConeAngle = 360.f;
	float outerConeGain = 0.f;

	bool isDirectional = false;
};

//---------------------------------
// AudioSource3DComponent
//
// Component data for audio sources that should be updated relative to the entities transform (instead of globally)
//  - must be attatched to entities with a transform component and an audio source component
//
struct AudioSource3DComponent final
{
	// definitions
	//---------------
	ECS_DECLARE_COMPONENT

	friend class AudioSourceSystem;

	// modifiers
	//-----------
public:
	void SetParams(AudioSource3DParams const& params) { m_Params = params; m_UpdateParams = true; }

	// Data
	///////

private:
	vec3 m_PrevPos;
	AudioSource3DParams m_Params;
	bool m_UpdateParams = true;
};

//---------------------------------
// AudioSourceCompDesc
//
// Descriptor for serialization and deserialization of audio source components
//
class AudioSourceCompDesc final : public ComponentDescriptor<AudioSourceComponent, true>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<AudioSourceComponent, true>)

	// construct destruct
	//--------------------
public:
	AudioSourceCompDesc() : ComponentDescriptor<AudioSourceComponent, true>() {}
	~AudioSourceCompDesc() = default;

	// ComponentDescriptor interface
	//-------------------------------
	AudioSourceComponent* MakeData() override;
	void OnScenePostLoad(EcsController& ecs, T_EntityId const id, AudioSourceComponent& comp);

	// Data
	///////

	AssetPtr<AudioData> audioAsset;

	bool playOnInit = false;
	bool loop = false;

	float gain = 1.f;
	float minGain = 0.f;
	float maxGain = 1.f;
	float pitch = 1.f;

	AudioSource3DParams params; // #todo: once deserialization supports null pointers, make it a pointer
};


} // namespace fw
} // namespace et
