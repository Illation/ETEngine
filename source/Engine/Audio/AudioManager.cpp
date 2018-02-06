#include "stdafx.hpp"
#include "AudioManager.h"

#define TEST_ERROR(_msg)				\
	error = alGetError();				\
	if (error != AL_NO_ERROR) {			\
		fprintf(stderr, _msg "\n");		\
		return;							\
	}

void AudioManager::Initialize()
{
	m_Device = alcOpenDevice(NULL);
	if (!m_Device)
	{
		std::cout << "Error creating openAL device" << std::endl;
		return;
	}
	ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if (enumeration = AL_FALSE)
	{
		std::cout << "openAL enumeration not supported" << std::endl;
	}

	ListAudioDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	const ALCchar *defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

	m_Device = alcOpenDevice(defaultDeviceName);
	if (!m_Device) 
	{
		std::cout << "Error unable to open default openAL device" << std::endl;
		return;
	}
	ALCenum error;

	fprintf(stdout, "Device: %s\n", alcGetString(m_Device, ALC_DEVICE_SPECIFIER));
	alGetError();

	m_Context = alcCreateContext(m_Device, NULL);
	if (!alcMakeContextCurrent(m_Context))
	{
		std::cout << "Error openAL failed to make default context" << std::endl;
		return;
	}
	TEST_ERROR("make default context");
}

void AudioManager::Destroy()
{
	m_Device = alcGetContextsDevice(m_Context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(m_Context);
	alcCloseDevice(m_Device);
}

bool AudioManager::TestALError(std::string error)
{
	ALCenum alerr = alGetError();				
	if (alerr != AL_NO_ERROR)
	{
		fprintf(stderr, (error + "\n").c_str());		
		return true;							
	}
	return false;
}

void AudioManager::SetDistanceModel(ALenum model)
{
	alDistanceModel(model);
	TestALError("Error setting openAL distance model");
}

AudioManager::~AudioManager()
{
}

void AudioManager::ListAudioDevices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	fprintf(stdout, "Devices list:\n");
	fprintf(stdout, "----------\n");
	while (device && *device != '\0' && next && *next != '\0') {
		fprintf(stdout, "%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	fprintf(stdout, "----------\n");
}
