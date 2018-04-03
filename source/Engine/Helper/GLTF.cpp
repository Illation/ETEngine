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

	//Rest of the data we are interested in
	if (!ParseSceneJson(json, dom))
	{
		LOG("Failed to parse scene from JSON", Warning);
		return false;
	}
	if (!ParseScenesJson(json, dom.scenes))
	{
		LOG("Failed to parse scenes from JSON", Warning);
		return false;
	}
	if (!ParseNodesJson(json, dom.nodes))
	{
		LOG("Failed to parse nodes from JSON", Warning);
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

bool glTF::ParseSceneJson(JSON::Object* root, Dom& dom)
{
	JSON::Value* sceneVal = (*root)["scene"];
	if (sceneVal)
	{
		if (!(sceneVal->GetType() == JSON::ValueType::JSON_Number)) return false;
		JSON::Number* sceneNum = sceneVal->num();
		if (!(sceneNum->isInt))return false;
		dom.scene = static_cast<int32>(sceneNum->valueInt);
		return true;
	}
	return true;
}

bool glTF::ParseScenesJson(JSON::Object* root, std::vector<Scene>& scenes)
{
	JSON::Value* scenesVal = (*root)["scenes"];
	if (!scenesVal)return true;

	if (!(scenesVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* scenesArr = scenesVal->arr();

	for (JSON::Value* sceneVal : scenesArr->value)
	{
		if (!(sceneVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* sceneObj = sceneVal->obj();

		Scene scene;

		JSON::Value* nameVal = (*sceneObj)["name"];
		if (nameVal)
		{
			if (!(nameVal->GetType() == JSON::ValueType::JSON_String)) return false;
			scene.name = nameVal->str()->value;
		}

		JSON::Value* nodesVal = (*sceneObj)["nodes"];
		if (nodesVal)
		{
			if (!(nodesVal->GetType() == JSON::ValueType::JSON_Array)) return false;
			JSON::Array* nodesArr = nodesVal->arr();
			std::vector<int64> intArr = nodesArr->IntArr();
			for (auto el : intArr) scene.nodes.push_back(static_cast<uint32>(el));
		}

		scenes.push_back(scene);
	}

	return true;
}

bool glTF::ParseNodesJson(JSON::Object* root, std::vector<Node>& nodes)
{
	JSON::Value* nodesVal = (*root)["nodes"];
	if (!nodesVal)return true;

	if (!(nodesVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* nodesArr = nodesVal->arr();

	for (JSON::Value* nodeVal : nodesArr->value)
	{
		if (!(nodeVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* nodeObj = nodeVal->obj();

		Node node;

		JSON::Value* nameVal = (*nodeObj)["name"];
		if (nameVal)
		{
			if (!(nameVal->GetType() == JSON::ValueType::JSON_String)) return false;
			node.name = nameVal->str()->value;
		}

		JSON::Value* childrenVal = (*nodeObj)["children"];
		if (childrenVal)
		{
			if (!(childrenVal->GetType() == JSON::ValueType::JSON_Array)) return false;
			JSON::Array* childrenArr = childrenVal->arr();
			std::vector<int64> intArr = childrenArr->IntArr();
			for (auto el : intArr) node.children.push_back(static_cast<uint32>(el));
		}

		JSON::Value* translationVal = (*nodeObj)["translation"];
		if (translationVal)
		{
			if (!JSON::ArrayVector(translationVal, node.translation))return false;
		}

		JSON::Value* rotationVal = (*nodeObj)["rotation"];
		if (rotationVal)
		{
			if (!JSON::ArrayVector(rotationVal, node.rotation.v4))return false;
		}

		JSON::Value* scaleVal = (*nodeObj)["scale"];
		if (scaleVal)
		{
			if (!JSON::ArrayVector(scaleVal, node.scale))return false;
		}

		bool hasMatrix = false;
		JSON::Value* matrixVal = (*nodeObj)["matrix"];
		if (matrixVal)
		{
			hasMatrix = true;
			if(!JSON::ArrayMatrix(matrixVal, node.matrix))return false;
		}

		if (hasMatrix)
		{
			// #todo: decompose matrix into TRS
		}
		else
		{
			node.matrix = etm::scale(node.scale) * etm::rotate(node.rotation) * etm::translate(node.translation);
		}

		JSON::Value* cameraVal = (*nodeObj)["camera"];
		if (cameraVal)
		{
			if (!(cameraVal->GetType() == JSON::ValueType::JSON_Number)) return false;
			JSON::Number* num = cameraVal->num();
			if (!(num->isInt))return false;
			node.camera = static_cast<int32>(num->valueInt);
		}

		JSON::Value* meshVal = (*nodeObj)["mesh"];
		if (meshVal)
		{
			if (!(meshVal->GetType() == JSON::ValueType::JSON_Number)) return false;
			JSON::Number* num = meshVal->num();
			if (!(num->isInt))return false;
			node.mesh = static_cast<int32>(num->valueInt);
		}

		JSON::Value* skinVal = (*nodeObj)["skin"];
		if (skinVal)
		{
			if (!(skinVal->GetType() == JSON::ValueType::JSON_Number)) return false;
			JSON::Number* num = skinVal->num();
			if (!(num->isInt))return false;
			node.skin = static_cast<int32>(num->valueInt);
		}

		JSON::Value* weightsVal = (*nodeObj)["weights"];
		if (weightsVal)
		{
			if (!(weightsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
			JSON::Array* weightsArr = weightsVal->arr();
			std::vector<double> arr = weightsArr->NumArr();
			for (auto el : arr) node.weights.push_back(static_cast<float>(el));
		}

		nodes.push_back(node);
	}

	return true;
}

void glTF::LogGLTFVersionSupport()
{
	LOG(std::string("glTF minVersion ") + std::to_string(glTF::minVersion) + " maxVersion " + std::to_string(glTF::maxVersion));
}
