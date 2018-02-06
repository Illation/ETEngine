#pragma once
#include "AbstractComponent.hpp"

class AudioListenerComponent : public AbstractComponent
{
public:
	AudioListenerComponent() {}
	virtual ~AudioListenerComponent() {}

	float GetGain() const { return m_Gain; }
	void SetGain(float val);

protected:
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override {}
	virtual void DrawForward() override {}

private:
	vec3 m_PrevPos = vec3(0);

	float m_Gain = 1;

	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	AudioListenerComponent(const AudioListenerComponent& obj);
	AudioListenerComponent& operator=(const AudioListenerComponent& obj);
};