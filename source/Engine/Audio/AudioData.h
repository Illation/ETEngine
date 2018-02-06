#pragma once
#include <AL\al.h>

class AudioData
{
public:
	AudioData(ALuint handle):m_Buffer(handle) {}
	virtual ~AudioData()
	{
		alDeleteBuffers(1, &m_Buffer);
	}

	ALuint GetHandle() { return m_Buffer; }

private:
	ALuint m_Buffer;
};