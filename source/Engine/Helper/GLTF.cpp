#include "stdafx.hpp"
#include "GLTF.h"

bool glTF::EvaluateURI(const std::string& uri, std::vector<uint8>& binData, std::string& ext)
{
	if (uri.substr(0, 5) == "data:")
	{
		auto dataPos = uri.find(',');
		if (dataPos == std::string::npos)
		{
			LOG("couldn't find data uri data", Warning);
			return false;
		}

		std::string mediatype = uri.substr(5, dataPos - 5);
		auto paramPos = mediatype.find(';');
		if (paramPos == std::string::npos)
		{
			LOG("couldn't find data uri mediatype parameter", Warning);
			return false;
		}
		std::string parameter = mediatype.substr(paramPos);
		mediatype = mediatype.substr(0, paramPos);

		auto subtypePos = mediatype.find('/');
		if (subtypePos == std::string::npos)
		{
			LOG("couldn't find data uri mediatype subtype", Warning);
			return false;
		}
		ext = mediatype.substr(subtypePos);
		mediatype = mediatype.substr(0, subtypePos);

		std::string dataString = uri.substr(dataPos);

		if (parameter == "base64")
		{
			if (DecodeBase64(dataString, binData))
			{
				return true;
			}
		}

		return false;
	}
	LOG("Loading uris from external files not implemented", Warning);
	return false;
}

bool glTF::DecodeBase64(const std::string& encoded, std::vector<uint8>& decoded)
{
	auto in_len = encoded.size();
	uint32 i = 0;
	uint8 char_array_4[4], char_array_3[3];

	while (in_len-- && (encoded[i] != '='))
	{
		if (!IsBase64(encoded[i]))return false;

		char_array_4[i%4] = static_cast<uint8>(Base64Mime.find(encoded[i]));
		++i;
		if (i%4 == 0) 
		{
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (uint32 j = 0; j < 3; j++)
			{
				decoded.push_back(char_array_3[j]);
			}
		}
	}

	if (i%4) 
	{
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

		for (uint32 j = 0; (j < i%4 - 1); j++) decoded.push_back(char_array_3[j]);
	}

	return true;
}
