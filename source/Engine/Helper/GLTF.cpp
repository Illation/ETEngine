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
		LOG("Failed to parse glTF scene from JSON", Warning);
		return false;
	}
	if (!ParseScenesJson(json, dom.scenes))
	{
		LOG("Failed to parse glTF scenes from JSON", Warning);
		return false;
	}
	if (!ParseNodesJson(json, dom.nodes))
	{
		LOG("Failed to parse glTF nodes from JSON", Warning);
		return false;
	}
	if (!ParseMeshesJson(json, dom.meshes))
	{
		LOG("Failed to parse glTF meshes from JSON", Warning);
		return false;
	}
	if (!ParseAccessorsJson(json, dom.accessors))
	{
		LOG("Failed to parse glTF accessors from JSON", Warning);
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
	
	if(!JSON::ApplyStrValue(assetObj, asset.version, "version")) return false;
	JSON::ApplyStrValue(assetObj, asset.minVersion, "minVersion");
	JSON::ApplyStrValue(assetObj, asset.generator, "generator");
	JSON::ApplyStrValue(assetObj, asset.copyright, "copyright");

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
	JSON::ApplyIntValue(root, dom.scene, "scene");
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

		JSON::ApplyStrValue(sceneObj, scene.name, "name");

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

		JSON::ApplyStrValue(nodeObj, node.name, "name");

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

		JSON::ApplyIntValue(nodeObj, node.camera, "camera");
		JSON::ApplyIntValue(nodeObj, node.mesh, "mesh");
		JSON::ApplyIntValue(nodeObj, node.skin, "skin");

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

bool glTF::ParseMeshesJson(JSON::Object* root, std::vector<Mesh>& meshes)
{
	JSON::Value* meshesVal = (*root)["meshes"];
	if (!meshesVal)return true;

	if (!(meshesVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* meshesArr = meshesVal->arr();

	for (JSON::Value* meshVal : meshesArr->value)
	{
		if (!(meshVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* meshObj = meshVal->obj();

		Mesh mesh;

		JSON::ApplyStrValue(meshObj, mesh.name, "name");

		JSON::Value* primitivesVal = (*meshObj)["primitives"];
		if (primitivesVal)
		{
			if (!(primitivesVal->GetType() == JSON::ValueType::JSON_Array)) return false;
			JSON::Array* primitivesArr = primitivesVal->arr();
			for (JSON::Value* primitiveVal : primitivesArr->value)
			{
				if (!(primitiveVal->GetType() == JSON::ValueType::JSON_Object)) return false;
				JSON::Object* primitiveObj = primitiveVal->obj();
				Primitive prim;
				if (ParsePrimitiveJson(primitiveObj, prim))
				{
					mesh.primitives.push_back(prim);
				}
				else return false;
			}
		}

		JSON::Value* weightsVal = (*meshObj)["weights"];
		if (weightsVal)
		{
			if (!(weightsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
			JSON::Array* nodesArr = weightsVal->arr();
			std::vector<double> weightArr = nodesArr->NumArr();
			for (auto el : weightArr) mesh.weights.push_back(static_cast<float>(el));
		}

		meshes.push_back(mesh);
	}

	return true;
}

bool glTF::ParsePrimitiveJson(JSON::Object* primitiveObj, Primitive& primitive)
{
	JSON::Value* attributesVal = (*primitiveObj)["attributes"];
	if (!attributesVal)return false;
	if (!(attributesVal->GetType() == JSON::ValueType::JSON_Object)) return false;
	JSON::Object* attributeObj = attributesVal->obj();
	JSON::ApplyIntValue(attributeObj, primitive.attributes.position, "POSITION");
	JSON::ApplyIntValue(attributeObj, primitive.attributes.normal, "NORMAL");
	JSON::ApplyIntValue(attributeObj, primitive.attributes.tangent, "TANGENT");
	JSON::ApplyIntValue(attributeObj, primitive.attributes.texcoord0, "TEXCOORD_0");
	JSON::ApplyIntValue(attributeObj, primitive.attributes.texcoord1, "TEXCOORD_1");
	JSON::ApplyIntValue(attributeObj, primitive.attributes.color0, "COLOR_0");
	JSON::ApplyIntValue(attributeObj, primitive.attributes.joints0, "JOINTS_0");
	JSON::ApplyIntValue(attributeObj, primitive.attributes.weights0, "WEIGHTS_0");

	JSON::ApplyIntValue(primitiveObj, primitive.indices, "indices");
	JSON::ApplyIntValue(primitiveObj, primitive.material, "material");
	JSON::ApplyIntValue(primitiveObj, primitive.mode, "mode");

	JSON::Value* targetsVal = (*primitiveObj)["targets"];
	if (targetsVal)
	{
		if (!(targetsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
		JSON::Array* targetsArr = targetsVal->arr();
		for (JSON::Value* targetVal : targetsArr->value)
		{
			if (!(targetVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* targetObj = targetVal->obj();

			Primitive::Targets target;

			JSON::ApplyIntValue(targetObj, target.position, "POSITION");
			JSON::ApplyIntValue(targetObj, target.normal, "NORMAL");
			JSON::ApplyIntValue(targetObj, target.tangent, "TANGENT");

			primitive.targets.push_back(target);
		}
	}

	return true;
}

bool glTF::ParseAccessorsJson(JSON::Object* root, std::vector<Accessor>& accessors)
{
	JSON::Value* accessorsVal = (*root)["accessors"];
	if (!accessorsVal)return true;

	if (!(accessorsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* accessorsArr = accessorsVal->arr();

	for (JSON::Value* accessorVal : accessorsArr->value)
	{
		if (!(accessorVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* accessorObj = accessorVal->obj();

		Accessor accessor;

		//Required components
		if (!JSON::ApplyIntValue(accessorObj, accessor.componentType, "componentType"))return false;
		if (!JSON::ApplyIntValue(accessorObj, accessor.count, "count"))return false;
		std::string typeStr;
		if (!JSON::ApplyStrValue(accessorObj, typeStr, "type"))return false;
		bool typeFound = false;
		for (auto const& type : AccessorTypes)
		{
			if (type.second.second == typeStr)
			{
				accessor.type = type.first;
				typeFound = true;
				break;
			}
		}
		if (!typeFound)return false;

		//Rest
		JSON::ApplyIntValue(accessorObj, accessor.bufferView, "bufferView");
		JSON::ApplyIntValue(accessorObj, accessor.byteOffset, "byteOffset");
		JSON::ApplyBoolValue(accessorObj, accessor.normalized, "normalized");
		JSON::ApplyStrValue(accessorObj, accessor.name, "name");

		JSON::Value* maxVal = (*accessorObj)["max"];
		if (maxVal)
		{
			if (!(maxVal->GetType() == JSON::ValueType::JSON_Array)) return false;
			JSON::Array* jArr = maxVal->arr();
			std::vector<double> arr = jArr->NumArr();
			for (auto el : arr) accessor.max.push_back(static_cast<float>(el));
		}
		JSON::Value* minVal = (*accessorObj)["min"];
		if (minVal)
		{
			if (!(minVal->GetType() == JSON::ValueType::JSON_Array)) return false;
			JSON::Array* jArr = minVal->arr();
			std::vector<double> arr = jArr->NumArr();
			for (auto el : arr) accessor.min.push_back(static_cast<float>(el));
		}

		accessors.push_back(accessor);

		JSON::Value* sparseVal = (*accessorObj)["sparse"];
		if (sparseVal)
		{
			if (!(sparseVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* sparseObj = sparseVal->obj();

			accessors[accessors.size()-1].sparse = new Accessor::Sparse();
			Accessor::Sparse* sparse = accessors[accessors.size() - 1].sparse;

			if(!JSON::ApplyIntValue(sparseObj, sparse->count, "count"))return false;

			JSON::Value* indicesVal = (*sparseObj)["indices"];
			if (!indicesVal)return false;
			if (!(indicesVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* indicesObj = indicesVal->obj();
			if(!JSON::ApplyIntValue(indicesObj, sparse->indices.bufferView, "bufferView"))return false;
			JSON::ApplyIntValue(indicesObj, sparse->indices.byteOffset, "byteOffset");
			if (!JSON::ApplyIntValue(indicesObj, sparse->indices.componentType, "componentType"))return false;

			JSON::Value* valuesVal = (*sparseObj)["values"];
			if (!valuesVal)return false;
			if (!(valuesVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* valuesObj = valuesVal->obj();
			if (!JSON::ApplyIntValue(valuesObj, sparse->values.bufferView, "bufferView"))return false;
			JSON::ApplyIntValue(valuesObj, sparse->values.byteOffset, "byteOffset");
		}
	}
	return true;
}

void glTF::LogGLTFVersionSupport()
{
	LOG(std::string("glTF minVersion ") + std::to_string(glTF::minVersion) + " maxVersion " + std::to_string(glTF::maxVersion));
}
