#pragma once


//===================
// Audio Asset
//===================


//-------------------------------
// AudioAsset::ConvertToMono
//
// Templated implementation to convert a buffer to mono channel
//
template<class T_DataType>
void AudioAsset::ConvertToMono(AudioBufferData &bufferData)
{
	uint32 samples = (bufferData.size / sizeof(T_DataType)) / 2;

	T_DataType* origin = reinterpret_cast<T_DataType*>(bufferData.data);
	T_DataType* dest = new T_DataType[samples];

	for (uint32 i = 0; i < samples; ++i)
	{
		dest[i] = (origin[i * 2] + origin[(i * 2) + 1]) / 2;
	}

	delete[] origin;
	bufferData.data = dest;
	bufferData.size = bufferData.size / 2;
}
