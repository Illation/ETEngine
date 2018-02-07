#pragma once
#include "ContentLoader.hpp"
#include "Audio\AudioData.h"

class AudioLoader : public ContentLoader<AudioData>
{
public:
	AudioLoader() {}
	~AudioLoader() {}

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
};
