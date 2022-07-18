#include "stdafx.h"
#include "AudioManager.h"


namespace et {
namespace fw {


void AudioManager::Initialize()
{
	m_Device = alcOpenDevice(NULL);
	if (!m_Device)
	{
		ET_LOG_E(ET_CTX_FRAMEWORK, "Unable to create openAL device");
		return;
	}

	ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if (enumeration == AL_FALSE)
	{
		ET_LOG_W(ET_CTX_FRAMEWORK, "OpenAL enumeration not supported");
	}

	ListAudioDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	const ALCchar *defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

	m_Device = alcOpenDevice(defaultDeviceName);
	if (!m_Device) 
	{
		ET_LOG_E(ET_CTX_FRAMEWORK, "Unable to open default openAL device");
		return;
	}

	ET_LOG_I(ET_CTX_FRAMEWORK, "Chosen device: %s", alcGetString(m_Device, ALC_DEVICE_SPECIFIER));

	m_Context = alcCreateContext(m_Device, NULL);
	if (!alcMakeContextCurrent(m_Context))
	{
		ET_LOG_E(ET_CTX_FRAMEWORK, "OpenAL failed to make default context");
		return;
	}

	ALCenum error = alGetError();
	if (error != AL_NO_ERROR) 
	{
		ET_LOG_E(ET_CTX_FRAMEWORK, "OpenAL failed to make default context");
		return;
	}
	
	ET_LOG_I(ET_CTX_FRAMEWORK, "OpenAL loaded\n");
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

		ET_TRACE_E(ET_CTX_FRAMEWORK, "%s : %s", error.c_str(), alErrorString.c_str());
		return true;							
	}
	return false;
}

void AudioManager::SetDistanceModel(ALenum model)
{
	alDistanceModel(model);
	TestALError("Error setting openAL distance model");
}

void AudioManager::MakeContextCurrent()
{
	if (!alcMakeContextCurrent(m_Context))
	{
		ET_LOG_E(ET_CTX_FRAMEWORK, "OpenAL failed to make context current");
	}
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

	ET_LOG_I(ET_CTX_FRAMEWORK, "OpenAL device list:");
	while (device && *device != '\0' && next && *next != '\0') 
	{
		ET_LOG_I(ET_CTX_FRAMEWORK, "\t%s", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
}


} // namespace fw
} // namespace et
