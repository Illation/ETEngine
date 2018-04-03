#include "stdafx.hpp"
#include "GLTF.h"
#include "FileSystem/Entry.h"
#include "FileSystem/JSONparser.h"
#include "FileSystem/JSONdom.h"

bool glTF::EvaluateURI(const std::string& uri, std::vector<uint8>& binData, std::string& ext, const std::string& basePath)
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
		std::string parameter = mediatype.substr(paramPos + 1);
		mediatype = mediatype.substr(0, paramPos);

		auto subtypePos = mediatype.find('/');
		if (subtypePos == std::string::npos)
		{
			LOG("couldn't find data uri mediatype subtype", Warning);
			return false;
		}
		ext = mediatype.substr(subtypePos + 1);
		mediatype = mediatype.substr(0, subtypePos);

		std::string dataString = uri.substr(dataPos + 1);

		if (parameter == "base64")
		{
			if (DecodeBase64(dataString, binData))
			{
				return true;
			}
		}

		return false;
	}
	else
	{
		Directory* pDir = new Directory(basePath, nullptr);
		File* input = new File(uri, pDir);
		if (!input->Open(FILE_ACCESS_MODE::Read))
		{
			LOG(std::string("Unable to open external glTF asset") + uri, Warning);
			return false;
		}
		binData = input->Read();
		ext = input->GetExtension();
		delete input;
		input = nullptr;
		delete pDir;
		pDir = nullptr;
		if (binData.size() == 0)
		{
			LOG(std::string("external glTF asset is empty") + uri, Warning);
		}
		return true;
	}
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

bool glTF::ParseGlTFJson(JSON::Object* json, Dom& dom)
{
	//Minimum requirement: asset and version
	if (!ParseAssetJson(json, dom.asset))
	{
		LOG("Failed to parse assset from JSON, invalid glTF file", Warning);
		return false;
	}
	//Also parse extensions and make sure we support them
	if (!ParseExtensionsJson(json, dom))
	{
		LOG("Failed to parse extensions from JSON, invalid glTF file", Warning);
		return false;
	}

	return true;
}

bool glTF::ParseAssetJson(JSON::Object* root, Asset& asset)
{
	JSON::Value* assetVal = (*root)["asset"];
	if (!assetVal)return false;
	if (!(assetVal->GetType() == JSON::ValueType::JSON_Object)) return false;
	JSON::Object* assetObj = assetVal->obj();
	
	JSON::Value* versionVal = (*assetObj)["version"];
	if (!versionVal)return false;
	if (!(versionVal->GetType() == JSON::ValueType::JSON_String)) return false;
	asset.version = versionVal->str()->value;

	JSON::Value* minVersionVal = (*assetObj)["minVersion"];
	if (minVersionVal)
	{
		if (!(minVersionVal->GetType() == JSON::ValueType::JSON_String)) return false;
		asset.minVersion = minVersionVal->str()->value;
	}

	JSON::Value* generatorVal = (*assetObj)["generator"];
	if (generatorVal)
	{
		if (!(generatorVal->GetType() == JSON::ValueType::JSON_String)) return false;
		asset.generator = generatorVal->str()->value;
	}

	JSON::Value* copyrightVal = (*assetObj)["copyright"];
	if (copyrightVal)
	{
		if (!(copyrightVal->GetType() == JSON::ValueType::JSON_String)) return false;
		asset.copyright = copyrightVal->str()->value;
	}

	//Check version support
	bool hasMinVersion = false;
	if (asset.minVersion.size())
	{
		hasMinVersion = true;
		float minAssetVersion = std::stof(asset.minVersion);
		if (minAssetVersion < glTF::minVersion || minAssetVersion > glTF::maxVersion)
		{
			LOG("glTF minVersion is not supported by ETEngine", Warning);
			LogGLTFVersionSupport();
			return false;
		}
	}
	float version = std::stof(asset.version);
	if (version < glTF::minVersion)
	{
		LOG("glTF version is too low not supported by ETEngine", Warning);
		LogGLTFVersionSupport();
		return false;
	}
	if (!hasMinVersion)
	{
		if (version > glTF::maxVersion)
		{
			LOG("glTF version is too high not supported by ETEngine", Warning);
			LogGLTFVersionSupport();
			return false;
		}
	}

	return true;
}

bool glTF::ParseExtensionsJson(JSON::Object* root, Dom& dom)
{
	JSON::Value* extUsedVal = (*root)["extensionsRequired"];
	if (extUsedVal)
	{
		if (!(extUsedVal->GetType() == JSON::ValueType::JSON_Array)) return false;
		dom.extensionsUsed = extUsedVal->arr()->StrArr();
	}

	JSON::Value* extRequVal = (*root)["extensionsRequired"];
	if (extRequVal)
	{
		if (!(extRequVal->GetType() == JSON::ValueType::JSON_Array)) return false;
		dom.extensionsRequired = extRequVal->arr()->StrArr();
	}

	//Check extension support
	for (auto requ : dom.extensionsRequired)
	{
		if (std::find(dom.extensionsUsed.begin(), dom.extensionsUsed.end(), requ) == dom.extensionsUsed.end())
		{
			LOG(std::string("Required glTF extension '") + requ + "' not found in used extensions", Warning);
			return false;
		}
		if (std::find(supportedExtensions.begin(), supportedExtensions.end(), requ) == supportedExtensions.end())
		{
			LOG(std::string("Required glTF extension '") + requ + "' is not supported by ETEngine", Warning);
			return false;
		}
	}
	return true;
}

void glTF::LogGLTFVersionSupport()
{
	LOG(std::string("glTF minVersion ") + std::to_string(glTF::minVersion) + " maxVersion " + std::to_string(glTF::maxVersion));
}
