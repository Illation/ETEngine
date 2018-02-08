#include "stdafx.hpp"
#include "AudioManager.h"

void AudioManager::Initialize()
{
	m_Device = alcOpenDevice(NULL);
	if (!m_Device)
	{
		Logger::Log("Unable to create openAL device", Error);
		return;
	}
	ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if (enumeration == AL_FALSE)
	{
		Logger::Log("OpenAL enumeration not supported", Warning);
	}

	ListAudioDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	const ALCchar *defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

	m_Device = alcOpenDevice(defaultDeviceName);
	if (!m_Device) 
	{
		Logger::Log("Unable to open default openAL device", Error);
		return;
	}
	ALCenum error;

	Logger::Log(std::string("Chosen device: ") + alcGetString(m_Device, ALC_DEVICE_SPECIFIER));
	alGetError();

	m_Context = alcCreateContext(m_Device, NULL);
	if (!alcMakeContextCurrent(m_Context))
	{
		Logger::Log("OpenAL failed to make default context", Error);
		return;
	}
	error = alGetError();				
	if (error != AL_NO_ERROR) 
	{
		Logger::Log("OpenAL failed to make default context", Error);
		return;
	}
	Logger::Log("OpenAL loaded\n");
}

bool AudioManager::TestALError(std::string error)
{
	ALCenum alerr = alGetError();				
	if (alerr != AL_NO_ERROR)
	{
		std::string alErrorString;
		switch (alerr) 
		{
		case AL_NO_ERROR: alErrorString = "AL_NO_ERROR"; break;
		case AL_INVALID_NAME: alErrorString = "AL_INVALID_NAME";break;
		case AL_INVALID_ENUM: alErrorString = "AL_INVALID_ENUM";break;
		case AL_INVALID_VALUE: alErrorString = "AL_INVALID_VALUE";break;
		case AL_INVALID_OPERATION: alErrorString = "AL_INVALID_OPERATION";break;
		case AL_OUT_OF_MEMORY: alErrorString = "AL_OUT_OF_MEMORY";break;
		default:
			alErrorString = "Unknown error code";break;
		}
		Logger::Log(error + " : " + alErrorString, Error);
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
	m_Device = alcGetContextsDevice(m_Context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(m_Context);
	alcCloseDevice(m_Device);
}

void AudioManager::ListAudioDevices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	Logger::Log("OpenAL device list:");
	while (device && *device != '\0' && next && *next != '\0') 
	{
		Logger::Log(std::string("\t")+device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
}
