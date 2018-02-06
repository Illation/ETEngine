#pragma once
#include "AbstractComponent.hpp"
#include <AL/al.h>

class AudioData;

// #todo: handle streaming and relative sources

class AudioSourceComponent : public AbstractComponent
{
public:
	AudioSourceComponent() {}
	virtual ~AudioSourceComponent();

	void SetAudioData(AudioData* val);

	float GetGain() const { return m_Gain; }
	void SetGain(float val);
	void SetMinGain(float val);
	void SetMaxGain(float val);

	float GetPitch() const { return m_Pitch; }
	void SetPitch(float val);

	bool IsLooping() const { return m_IsLooping; }
	void SetLooping(bool val);

	void Play();//Plays as soon as there is a source and audio data
	void Pause();
	void Stop();

	bool IsPlaying() const { return m_IsPlaying; }

	void SetIsDirectional(bool val);
	void SetInnerConeAngle(float val);
	void SetOuterConeAngle(float val);
	void SetOuterConeGain(float val);

	void SetReferenceDistance(float val);
	void SetRolloffFactor(float val);
	void SetMaxDistance(float val);

protected:
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override {}
	virtual void DrawForward() override {}

private:
	vec3 m_PrevPos = vec3(0);

	//Data
	AudioData* m_pAudioData = nullptr;

	//Sound
	float m_Gain = 1;
	float m_MinGain = 0;
	float m_MaxGain = 1;
	float m_Pitch = 1;

	//Direction
	bool m_IsDirectional = false;
	float m_InnerConeAngle = 360.f;
	float m_OuterConeAngle = 360.f;
	float m_OuterConeGain = 0.f;

	//Distance
	float m_ReferenceDistance = 1.f;
	float m_RolloffFactor = 1.f;
	float m_MaxDistance = std::numeric_limits<float>::max();

	//Playback
	bool m_IsLooping = false;

	bool m_IsPlaying = false;
	bool m_IsPaused = false;

	//OpenAL stuff
	ALuint m_Source;
	bool m_IsInitialized = false;

	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	AudioSourceComponent(const AudioSourceComponent& obj);
	AudioSourceComponent& operator=(const AudioSourceComponent& obj);
};