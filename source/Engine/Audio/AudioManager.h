#pragma once
#include "Singleton.hpp"

#include <AL/al.h>
#include <AL/alc.h>

class AudioManager : public Singleton<AudioManager>
{
public:
	void Initialize();
	void Destroy();

	bool TestALError(std::string error);

	void SetDistanceModel(ALenum model);

private:
	void ListAudioDevices(const ALCchar *devices);

	ALCdevice* m_Device;
	ALCcontext *m_Context;

private:
	friend class Singleton<AudioManager>;
	AudioManager() {}
	virtual ~AudioManager();
};