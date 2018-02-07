#pragma once
#include "ContentLoader.hpp"
#include "../Audio/AudioData.h"

class AudioLoader : public ContentLoader<AudioData>
{
public:
	AudioLoader() {}
	~AudioLoader() {}

	void ForceMono(bool val) { m_ForceMono = val; }

protected:
	virtual AudioData* LoadContent(const std::string& assetFile) override;
	virtual void Destroy(AudioData* objToDestroy) override;

private:
	struct AudioBufferData
	{
		ALenum format;
		ALvoid *data;
		ALsizei size;
		ALsizei frequency;
	};
	bool LoadWavFile(AudioBufferData &bufferData, const std::vector<uint8> &binaryContent);
	bool LoadOggFile(AudioBufferData &bufferData, const std::vector<uint8> &binaryContent);

	void ConvertToMono(AudioBufferData &bufferData);

	template<class T>
	void ConvertToMono(AudioBufferData &bufferData)
	{
		uint32 samples = (bufferData.size / sizeof(T)) / 2;

		T* origin = reinterpret_cast<T*>(bufferData.data);
		T* dest = new T[samples];

		for (uint32 i = 0; i < samples; ++i)
		{
			dest[i] = (origin[i * 2] + origin[(i * 2) + 1]) / 2;
		}

		delete[] origin;
		bufferData.data = dest;
		bufferData.size = bufferData.size / 2;
	}

	bool m_ForceMono = false;
};
