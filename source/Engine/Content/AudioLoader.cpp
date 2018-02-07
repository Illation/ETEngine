#include "stdafx.hpp"
#include "AudioLoader.h"
#include "FileSystem\Entry.h"
#include "Audio\AudioManager.h"

#include "staticDependancies/stb/stb_vorbis.h"
#include <array>

AudioData* AudioLoader::LoadContent(const std::string& assetFile)
{
	cout << "Loading Audio: " << assetFile << " . . . ";

	File* input = new File(assetFile, nullptr);
	if (!input->Open(FILE_ACCESS_MODE::Read))
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Opening font descriptor file failed." << endl;
		return nullptr;
	}
	std::vector<uint8> binaryContent = input->Read();
	std::string extension = input->GetExtension();
	delete input;
	input = nullptr;

	bool dataLoaded = false;
	AudioBufferData data;
	
	if		(extension == "wav") dataLoaded = LoadWavFile(data, binaryContent);
	else if (extension == "ogg") dataLoaded = LoadOggFile(data, binaryContent);
	else
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Cannot load audio data with this extension. Supported exensions:" << endl;
		cout << "        wav" << endl;
		cout << "        ogg" << endl;
		return nullptr;
	}
	if (!dataLoaded)
	{
		cout << "  . . . FAILED!" << endl;
		cout << "    Failed to load audio buffer data." << endl;
		return nullptr;
	}

	ALuint buffer;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, data.format, data.data, data.size, data.frequency);
	if (AudioManager::GetInstance()->TestALError("Audio Loader alBufferData error"))
	{
		cout << "  . . . FAILED!" << endl;
		return nullptr;
	}

	cout << "  . . . SUCCESS!" << endl;
	return new AudioData(buffer);
}

void AudioLoader::Destroy(AudioData* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}

bool AudioLoader::LoadWavFile(AudioBufferData &bufferData, const std::vector<uint8> &binaryContent)
{
	UNUSED(bufferData);
	UNUSED(binaryContent);
	cout << "AudioLoader::LoadWavFile > not implemented" << endl;
	return false;
}

bool AudioLoader::LoadOggFile(AudioBufferData &bufferData, const std::vector<uint8> &binaryContent)
{
	int e = 0;
	stb_vorbis* vorbis = stb_vorbis_open_memory(binaryContent.data(), (int)binaryContent.size(), &e, NULL);
	if(!vorbis) return false;
	if (e)
	{
		stb_vorbis_get_error(vorbis);
		stb_vorbis_close(vorbis);
		return false;
	}

	stb_vorbis_info info = stb_vorbis_get_info(vorbis);

	if (info.channels == 2)bufferData.format = AL_FORMAT_STEREO16;
	else bufferData.format = AL_FORMAT_MONO16;
	
	int32 samples = stb_vorbis_stream_length_in_samples(vorbis);
	const int32 bufferSize = samples * info.channels;

	std::vector<ALshort> pcm(bufferSize);
	int32  size = 0;
	int32  result = 0;

	while (size < bufferSize)
	{
		result = stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, pcm.data() + size, bufferSize - size);
		if (result > 0) size += result*info.channels;
		else break;
	}
	if (size == 0)
	{
		stb_vorbis_close(vorbis);
		return false;
	}

	bufferData.data = pcm.data();
	bufferData.size = size * sizeof(ALshort);
	bufferData.frequency = info.sample_rate;

	stb_vorbis_close(vorbis);

	return true;
}
