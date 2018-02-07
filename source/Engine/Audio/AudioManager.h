#pragma once
#include "../Helper/Singleton.hpp"

#include <AL/al.h>
#include <AL/alc.h>

class AudioManager : public Singleton<AudioManager>
{
public:
	void Initialize();

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