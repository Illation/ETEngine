#pragma warning(disable:4996)
#include "stdafx.hpp"
#include "AudioLoader.h"
#include "FileSystem\Entry.h"
#include "Audio\AudioManager.h"

#include "staticDependancies/stb/stb_vorbis.h"
#include <array>
#include "FileSystem\BinaryReader.hpp"

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
	delete[] data.data;

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
	auto pBinReader = new BinaryReader(); 

#define EXIT_FALSE {delete pBinReader;return false;}

	pBinReader->Open(binaryContent);
	if (!pBinReader->Exists()) EXIT_FALSE;

	//Read RIFF chunk descriptor
	std::string chunkID;
	for (uint8 i = 0; i < 4; i++)chunkID += pBinReader->Read<char>();
	if (chunkID != "RIFF") EXIT_FALSE;
	pBinReader->Read<uint32>();//chunk size
	std::string format;
	for (uint8 i = 0; i < 4; i++)format += pBinReader->Read<char>();
	if (format != "WAVE") EXIT_FALSE;

	//Read fmt subchunk
	std::string subChunk1ID;
	for (uint8 i = 0; i < 4; i++)subChunk1ID += pBinReader->Read<char>();
	if (subChunk1ID != "fmt ") EXIT_FALSE;
	if (pBinReader->Read<uint32>() != 16) EXIT_FALSE;//SubChunk1Size
	uint16 audioFormat = pBinReader->Read<uint16>();
	if (audioFormat != 1)
	{
		std::cout << "Only uncompressed wave files are supported, audio format is: " << audioFormat << std::endl;
		EXIT_FALSE;
	}
	uint16 numChannels = pBinReader->Read<uint16>();
	uint32 sampleRate = pBinReader->Read<uint32>();
	pBinReader->Read<uint32>();//byte rate
	pBinReader->Read<uint16>();//block align
	uint16 bitsPerSample = pBinReader->Read<uint16>();

	//Read data subchunk
	std::string subChunk2ID;
	for (uint8 i = 0; i < 4; i++)subChunk2ID += pBinReader->Read<char>();
	if (subChunk2ID != "data") EXIT_FALSE;
	uint32 subChunk2Size = pBinReader->Read<uint32>();
	int32 bufferPos = pBinReader->GetBufferPosition();

	delete pBinReader;
	pBinReader = nullptr;

	//Format into openAL buffer
	switch (numChannels)
	{
	case 1:
		switch (bitsPerSample)
		{
		case 8:bufferData.format = AL_FORMAT_MONO8; break;
		case 16:bufferData.format = AL_FORMAT_MONO16; break;
		default: std::cout << "only 8 and 16 bit formats are supported by openAL, bitSize: " << bitsPerSample << std::endl; EXIT_FALSE;
		} break;
	case 2:
		switch (bitsPerSample)
		{
		case 8:bufferData.format = AL_FORMAT_STEREO8; break;
		case 16:bufferData.format = AL_FORMAT_STEREO16; break;
		default: std::cout << "only 8 and 16 bit formats are supported by openAL, bitSize: " << bitsPerSample << std::endl; EXIT_FALSE;
		} break;
	default: 
		std::cout << "Only mono and stereo supported by openAL, numChannels: " << numChannels << std::endl; 
		EXIT_FALSE;
	}
	uint8* data = new uint8[subChunk2Size];
	std::copy(binaryContent.begin() + bufferPos, binaryContent.begin() + bufferPos + subChunk2Size, data);
	bufferData.data = data;
	bufferData.size = subChunk2Size;
	bufferData.frequency = sampleRate;

	return true;
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
#pragma warning(default:4996)