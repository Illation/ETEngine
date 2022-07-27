#include "stdafx.h"
#include "Base64.h"


namespace et {
namespace core {


//=============================
// Base 64 Encoding / Decoding
//=============================


namespace base64 {


std::string const s_Base64Mime("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");


//--------
// Encode
//
// Convert random binary data into a Base64 string
//
std::string Encode(std::vector<uint8> const& data)
{
	std::string ret;
	ret.reserve(data.size() + (data.size() / 2u)); // typically Encoded strings are about 1.33 times bigger

	size_t inLength = data.size();
	uint32 i = 0u;
	size_t in = 0u;
	uint8 charArray4[4], charArray3[3];

	while (inLength--)
	{
		charArray3[i++] = data[in];
		in++;

		if (i == 3)
		{
			charArray4[0] = (charArray3[0] & 0xFC) >> 2;
			charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xF0) >> 4);
			charArray4[2] = ((charArray3[1] & 0x0F) << 2) + ((charArray3[2] & 0xC0) >> 6);
			charArray4[3] = (charArray3[2] & 0xc3F);

			for (i = 0; i < 4; ++i)
			{
				ret += s_Base64Mime[charArray4[i]];
			}

			i = 0;
		}
	}

	if (i != 0)
	{
		uint32 j = 0u;
		for (j = i; j < 3; ++j)
		{
			charArray3[j] = '\0';
		}

		charArray4[0] = (charArray3[0] & 0xFC) >> 2;
		charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xF0) >> 4);
		charArray4[2] = ((charArray3[1] & 0x0F) << 2) + ((charArray3[2] & 0xC0) >> 6);

		for (j = 0u; j < i + 1; j++)
		{
			ret += s_Base64Mime[charArray4[j]];
		}

		while (i++ < 3)
		{
			ret += '=';
		}
	}

	return ret;
}

//--------
// Decode
//
// Convert a Base64 string into regular bytes
//
bool Decode(std::string const& encoded, std::vector<uint8>& decoded)
{
	size_t inLength = encoded.size();
	uint32 i = 0u;
	uint8 charArray4[4], charArray3[3];

	decoded.reserve(encoded.size());

	while (inLength-- && (encoded[i] != '='))
	{
		if (!IsBase64(encoded[i]))
		{
			return false;
		}

		charArray4[i % 4] = static_cast<uint8>(s_Base64Mime.find(encoded[i]));
		if (++i % 4 == 0)
		{
			charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
			charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
			charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

			for (uint32 j = 0; j < 3; j++)
			{
				decoded.push_back(charArray3[j]);
			}
		}
	}

	if (i % 4)
	{
		charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
		charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);

		for (uint32 j = 0; (j < i % 4 - 1); j++)
		{
			decoded.push_back(charArray3[j]);
		}
	}

	return true;
}


} // namespace base64


} // namespace core
} // namespace et
