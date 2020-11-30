#pragma once
#include <AL\al.h>
#include <rttr/type>

#include <EtCore/Content/Asset.h>
#include <EtCore/Util/LinkerUtils.h>


namespace et {
	REGISTRATION_NS(fw);
}


namespace et {
namespace fw {


//---------------------------------
// AudioData
//
// Minimal data required for openAL to play a sound effect
//
class AudioData final
{
	REGISTRATION_FRIEND_NS(fw)

	AudioData() = default;
public:
	AudioData(ALuint handle);
	virtual ~AudioData();

	ALuint GetHandle() const { return m_Buffer; }

private:
	ALuint m_Buffer = 0u;
};

//---------------------------------
// AudioAsset
//
// Loadable Audio Data
//
class AudioAsset final : public core::Asset<AudioData, false>
{
	DECLARE_FORCED_LINKING()
private:
	// definitions
	//---------------------

	//---------------------------------
	// AudioAsset::AudioBufferData
	//
	// Helper struct to pass around during load
	//
	struct AudioBufferData
	{
		ALenum format;
		ALvoid *data;
		ALsizei size;
		ALsizei frequency;
	};

public:
	// Construct destruct
	//---------------------
	AudioAsset() : core::Asset<AudioData, false>() {}
	virtual ~AudioAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Utility
	//---------------------
private:

	bool LoadWavFile(AudioBufferData &bufferData, std::vector<uint8> const& binaryContent);
	bool LoadOggFile(AudioBufferData &bufferData, std::vector<uint8> const& binaryContent);

	void ConvertToMono(AudioBufferData &bufferData);

	template<class T_DataType>
	void ConvertToMono(AudioBufferData &bufferData);

	// Data
	///////
public:
	bool m_IsMonoForced = false;

	RTTR_ENABLE(core::Asset<AudioData, false>)
};


} // namespace fw
} // namespace et


#include "AudioData.inl"
