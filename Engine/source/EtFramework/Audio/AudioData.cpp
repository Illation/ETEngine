#include "stdafx.h"
#include "AudioData.h"

#include "AudioManager.h"

#include <stb_vorbis.h>

#include <EtCore/Reflection/Registration.h>
#include <EtCore/FileSystem/BinaryReader.h>
#include <EtCore/FileSystem/FileUtil.h>


namespace et {
namespace fw {


//===================
// Audio Data
//===================


//---------------------------------
// AudioData::c-tor
//
// Construct audio data from a handle
//
AudioData::AudioData(ALuint handle) 
	: m_Buffer(handle)
{ }

//---------------------------------
// AudioData::d-tor
//
// Cleans up the OpenAL buffer
//
AudioData::~AudioData()
{
	alDeleteBuffers(1, &m_Buffer);
}


//===================
// Audio Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_POLYMORPHIC_CLASS(AudioAsset, "audio asset")
		.property("force mono", &AudioAsset::m_IsMonoForced)
	END_REGISTER_POLYMORPHIC_CLASS(AudioAsset, I_Asset);
}
DEFINE_FORCED_LINKING(AudioAsset) // force the asset class to be linked as it is only used in reflection


//---------------------------------
// AudioAsset::LoadFromMemory
//
// Load audio data from binary asset content
//
bool AudioAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	std::string extension = FileUtil::ExtractExtension(GetName());

	bool dataLoaded = false;
	AudioBufferData bufferData;

	if (extension == "wav")
	{
		dataLoaded = LoadWavFile(bufferData, data);
	}
	else if (extension == "ogg")
	{
		dataLoaded = LoadOggFile(bufferData, data);
	}
	else
	{
		LOG("AudioAsset::LoadFromMemory > Cannot load audio data with this extension! Supported exensions: [.wav/.ogg]", LogLevel::Warning);
		return false;
	}

	if (!dataLoaded)
	{
		LOG("AudioAsset::LoadFromMemory > Failed to load audio buffer data!", LogLevel::Warning);
		return false;
	}

	if (m_IsMonoForced)
	{
		ConvertToMono(bufferData);
	}

	ALuint buffer;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, bufferData.format, bufferData.data, bufferData.size, bufferData.frequency);

	if (AudioManager::GetInstance()->TestALError("Audio Loader alBufferData error"))
	{
		LOG("AudioAsset::LoadFromMemory > Failed - open AL error!", LogLevel::Warning);
		return false;
	}

	delete[] bufferData.data;

	m_Data = new AudioData(buffer);

	// all done
	return true;
}

//---------------------------------
// AudioAsset::LoadWavFile
//
// Read a WAV file from binary data
//
bool AudioAsset::LoadWavFile(AudioBufferData &bufferData, std::vector<uint8> const& binaryContent)
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
		LOG(std::string("Only uncompressed wave files are supported, audio format is: ") + std::to_string(audioFormat), Warning);
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
	uint32 bufferPos = (uint32)pBinReader->GetBufferPosition();

	delete pBinReader;
	pBinReader = nullptr;

	uint8* data = new uint8[subChunk2Size];
	for (uint32 i = 0; i < subChunk2Size; ++i)
	{
		if (i + bufferPos >= (uint32)binaryContent.size())
		{
			LOG("Unexpected end of wav files binary content", Warning);
			return false;
		}
		data[i] = binaryContent[i + bufferPos];
	}

	//Format into openAL buffer
	switch (numChannels)
	{
	case 1:
		switch (bitsPerSample)
		{
		case 8:bufferData.format = AL_FORMAT_MONO8; break;
		case 16:bufferData.format = AL_FORMAT_MONO16; break;
		default: LOG(std::string("only 8 and 16 bit formats are supported by openAL, bitSize: ") + std::to_string(bitsPerSample), Warning); EXIT_FALSE;
		} break;
	case 2:
		switch (bitsPerSample)
		{
		case 8:bufferData.format = AL_FORMAT_STEREO8; break;
		case 16:bufferData.format = AL_FORMAT_STEREO16; break;
		default: LOG(std::string("only 8 and 16 bit formats are supported by openAL, bitSize: ") + std::to_string(bitsPerSample), Warning); EXIT_FALSE;
		} break;
	default:
		LOG(std::string("Only mono and stereo supported by openAL, numChannels: ") + std::to_string(numChannels), Warning);
		EXIT_FALSE;
	}
	bufferData.data = data;
	bufferData.size = subChunk2Size;
	bufferData.frequency = sampleRate;

	return true;
}

//---------------------------------
// AudioAsset::LoadOggFile
//
// Read a OGG Vorbis file from binary data using STB Vorbis
//
bool AudioAsset::LoadOggFile(AudioBufferData &bufferData, std::vector<uint8> const& binaryContent)
{
	int e = 0;
	stb_vorbis* vorbis = stb_vorbis_open_memory(binaryContent.data(), (int)binaryContent.size(), &e, NULL);
	if (!vorbis) return false;
	if (e)
	{
		stb_vorbis_get_error(vorbis);
		stb_vorbis_close(vorbis);
		return false;
	}

	stb_vorbis_info info = stb_vorbis_get_info(vorbis);

	switch (info.channels)
	{
	case 1:bufferData.format = AL_FORMAT_MONO16; break;
	case 2:bufferData.format = AL_FORMAT_STEREO16; break;
	default:
		LOG(std::string("Only mono and stereo supported by openAL, numChannels: ") + std::to_string(info.channels), LogLevel::Warning);
		stb_vorbis_close(vorbis);
		return false;
	}

	int32 samples = stb_vorbis_stream_length_in_samples(vorbis);
	const int32 bufferSize = samples * info.channels;

	ALshort* pcm = new ALshort[bufferSize];
	int32  size = 0;
	int32  result = 0;

	while (size < bufferSize)
	{
		result = stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, pcm + size, bufferSize - size);
		if (result > 0) size += result * info.channels;
		else break;
	}
	if (size == 0)
	{
		stb_vorbis_close(vorbis);
		return false;
	}

	bufferData.data = pcm;
	bufferData.size = size * sizeof(ALshort);
	bufferData.frequency = info.sample_rate;

	stb_vorbis_close(vorbis);

	return true;
}

//---------------------------------
// AudioAsset::ConvertToMono
//
// Convert Audio buffer data into a single channel (for 3D audio sources)
//
void AudioAsset::ConvertToMono(AudioBufferData &bufferData)
{
	switch (bufferData.format)
	{
	case AL_FORMAT_STEREO8:
		{
			ConvertToMono<int8>(bufferData);
			bufferData.format = AL_FORMAT_MONO8;
		}
		break;
	case AL_FORMAT_STEREO16:
		{
			ConvertToMono<int16>(bufferData);
			bufferData.format = AL_FORMAT_MONO16;
		}
		break;
	}
}


} // namespace fw
} // namespace et
