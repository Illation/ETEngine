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
	if (!ParseBufferViewsJson(json, dom.bufferViews))
	{
		LOG("Failed to parse glTF bufferViews from JSON", Warning);
		return false;
	}
	if (!ParseBuffersJson(json, dom.buffers))
	{
		LOG("Failed to parse glTF buffers from JSON", Warning);
		return false;
	}
	if (!ParseImagesJson(json, dom.images))
	{
		LOG("Failed to parse glTF images from JSON", Warning);
		return false;
	}
	if (!ParseTexturesJson(json, dom.textures))
	{
		LOG("Failed to parse glTF textures from JSON", Warning);
		return false;
	}
	if (!ParseSamplersJson(json, dom.samplers))
	{
		LOG("Failed to parse glTF samplers from JSON", Warning);
		return false;
	}
	if (!ParseMaterialsJson(json, dom.materials))
	{
		LOG("Failed to parse glTF materials from JSON", Warning);
		return false;
	}
	if (!ParseCamerasJson(json, dom.cameras))
	{
		LOG("Failed to parse glTF cameras from JSON", Warning);
		return false;
	}
	if (!ParseSkinsJson(json, dom.skins))
	{
		LOG("Failed to parse glTF skins from JSON", Warning);
		return false;
	}
	if (!ParseAnimationsJson(json, dom.animations))
	{
		LOG("Failed to parse glTF animations from JSON", Warning);
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

		bool hasTranslation = false;
		JSON::Value* translationVal = (*nodeObj)["translation"];
		if (translationVal)
		{
			if (!JSON::ArrayVector(translationVal, node.translation))return false;
			hasTranslation = true;
		}

		bool hasRotation = false;
		JSON::Value* rotationVal = (*nodeObj)["rotation"];
		if (rotationVal)
		{
			if (!JSON::ArrayVector(rotationVal, node.rotation.v4))return false;
			hasRotation = true;
		}

		bool hasScale = false;
		JSON::Value* scaleVal = (*nodeObj)["scale"];
		if (scaleVal)
		{
			if (!JSON::ArrayVector(scaleVal, node.scale))return false;
			hasScale = true;
		}

		bool hasMatrix = false;
		JSON::Value* matrixVal = (*nodeObj)["matrix"];
		if (matrixVal)
		{
			if(!JSON::ArrayMatrix(matrixVal, node.matrix))return false;
			hasMatrix = true;
		}

		if (hasMatrix)
		{
			vec3 trans, scale;
			quat rot;
			etm::decomposeTRS(node.matrix, trans, rot, scale);
			if (!hasTranslation)node.translation = trans;
			else if (!etm::nearEqualsV(node.translation, trans, 0.0001f))LOG("inconsistent translation values for node", Warning);
			if (!hasRotation)node.rotation = rot;
			else
			{
				//different quaternions can express the same rotation
				vec3 testVec = vec3(0, 0, 1);
				if (!etm::nearEqualsV(node.rotation * testVec, rot * testVec, 0.0001f))LOG("inconsistent rotation values for node", Warning);
			}
			if (!hasScale)node.scale = scale;
			else if (!etm::nearEqualsV(node.scale, scale, 0.0001f))LOG("inconsistent scale values for node", Warning);
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

bool glTF::ParseBufferViewsJson(JSON::Object* root, std::vector<BufferView>& bufferViews)
{
	JSON::Value* bufferViewsVal = (*root)["bufferViews"];
	if (!bufferViewsVal)return true;

	if (!(bufferViewsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* bufferViewsArr = bufferViewsVal->arr();

	for (JSON::Value* bufferViewVal : bufferViewsArr->value)
	{
		if (!(bufferViewVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* bufferViewObj = bufferViewVal->obj();

		BufferView view;

		if(!JSON::ApplyIntValue(bufferViewObj, view.buffer, "buffer"))return false;
		if(!JSON::ApplyIntValue(bufferViewObj, view.byteLength, "byteLength"))return false;
		JSON::ApplyIntValue(bufferViewObj, view.byteOffset, "byteOffset");
		JSON::ApplyIntValue(bufferViewObj, view.byteStride, "byteStride");
		JSON::ApplyIntValue(bufferViewObj, view.target, "target");
		JSON::ApplyStrValue(bufferViewObj, view.name, "name");

		bufferViews.push_back(view);
	}

	return true;
}

bool glTF::ParseBuffersJson(JSON::Object* root, std::vector<Buffer>& buffers)
{
	JSON::Value* buffersVal = (*root)["buffers"];
	if (!buffersVal)return true;

	if (!(buffersVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* buffersArr = buffersVal->arr();

	for (JSON::Value* bufferVal : buffersArr->value)
	{
		if (!(bufferVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* bufferObj = bufferVal->obj();

		Buffer buffer;

		JSON::ApplyStrValue(bufferObj, buffer.uri, "uri");
		if (!JSON::ApplyIntValue(bufferObj, buffer.byteLength, "byteLength"))return false;
		JSON::ApplyStrValue(bufferObj, buffer.name, "name");

		buffers.push_back(buffer);
	}

	return true;
}

bool glTF::ParseTexturesJson(JSON::Object* root, std::vector<Texture>& textures)
{
	JSON::Value* texturesVal = (*root)["textures"];
	if (!texturesVal)return true;

	if (!(texturesVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* texturesArr = texturesVal->arr();

	for (JSON::Value* textureVal : texturesArr->value)
	{
		if (!(textureVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* textureObj = textureVal->obj();

		Texture texture;

		JSON::ApplyIntValue(textureObj, texture.sampler, "sampler");
		JSON::ApplyIntValue(textureObj, texture.source, "source");
		JSON::ApplyStrValue(textureObj, texture.name, "name");

		textures.push_back(texture);
	}

	return true;
}

bool glTF::ParseImagesJson(JSON::Object* root, std::vector<Image>& images)
{
	JSON::Value* imagesVal = (*root)["images"];
	if (!imagesVal)return true;

	if (!(imagesVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* imagesArr = imagesVal->arr();

	for (JSON::Value* imageVal : imagesArr->value)
	{
		if (!(imageVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* imageObj = imageVal->obj();

		Image image;

		JSON::ApplyStrValue(imageObj, image.uri, "uri");
		JSON::ApplyIntValue(imageObj, image.bufferView, "bufferView");
		JSON::ApplyStrValue(imageObj, image.mimeType, "mimeType");
		JSON::ApplyStrValue(imageObj, image.name, "name");

		images.push_back(image);
	}

	return true;
}

bool glTF::ParseSamplersJson(JSON::Object* root, std::vector<Sampler>& samplers)
{
	JSON::Value* samplersVal = (*root)["samplers"];
	if (!samplersVal)return true;

	if (!(samplersVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* samplersArr = samplersVal->arr();

	for (JSON::Value* samplerVal : samplersArr->value)
	{
		if (!(samplerVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* samplerObj = samplerVal->obj();

		Sampler sampler;

		JSON::ApplyIntValue(samplerObj, sampler.magFilter, "magFilter");
		JSON::ApplyIntValue(samplerObj, sampler.minFilter, "minFilter");
		JSON::ApplyIntValue(samplerObj, sampler.wrapS, "wrapS");
		JSON::ApplyIntValue(samplerObj, sampler.wrapT, "wrapT");
		JSON::ApplyStrValue(samplerObj, sampler.name, "name");

		samplers.push_back(sampler);
	}

	return true;
}

bool glTF::ParseMaterialsJson(JSON::Object* root, std::vector<Material>& materials)
{
	JSON::Value* materialsVal = (*root)["materials"];
	if (!materialsVal)return true;

	if (!(materialsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* materialsArr = materialsVal->arr();

	for (JSON::Value* materialVal : materialsArr->value)
	{
		if (!(materialVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* materialObj = materialVal->obj();

		materials.push_back(Material());
		Material* material = &materials[materials.size()-1];

		JSON::ApplyStrValue(materialObj, material->name, "name");

		if (material->pbrMetallicRoughness)return false;
		material->pbrMetallicRoughness = new Material::PbrMetallicRoughness();
		JSON::Value* pbrMetallicRoughnessVal = (*materialObj)["pbrMetallicRoughness"];
		if (pbrMetallicRoughnessVal)
		{
			if (!(pbrMetallicRoughnessVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			if (!ParsePbrMetallicRoughnessJson(pbrMetallicRoughnessVal->obj(), material->pbrMetallicRoughness))return false;
		}

		JSON::Value* normalTextureVal = (*materialObj)["normalTexture"];
		if (normalTextureVal)
		{
			if (!(normalTextureVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* normalTextureObj = normalTextureVal->obj();
			if (material->normalTexture)return false;
			material->normalTexture = new Material::NormalTextureInfo();
			if (!ParseTextureInfoJson(normalTextureObj, material->normalTexture))return false;
			if (material->normalTexture)
			{
				JSON::ApplyNumValue(normalTextureObj, material->normalTexture->scale, "scale");
			}
		}

		JSON::Value* occlusionTextureVal = (*materialObj)["occlusionTexture"];
		if (occlusionTextureVal)
		{
			if (!(occlusionTextureVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* occlusionTextureObj = occlusionTextureVal->obj();
			if (material->occlusionTexture)return false;
			material->occlusionTexture = new Material::OcclusionTextureInfo();
			if (!ParseTextureInfoJson(occlusionTextureObj, material->occlusionTexture))return false;
			if (material->occlusionTexture)
			{
				JSON::ApplyNumValue(occlusionTextureObj, material->occlusionTexture->strength, "strength");
			}
		}

		JSON::Value* emissiveTextureVal = (*materialObj)["emissiveTexture"];
		if (emissiveTextureVal)
		{
			if (!(emissiveTextureVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			if (material->emissiveTexture)return false;
			material->emissiveTexture = new Material::TextureInfo();
			if (!ParseTextureInfoJson(emissiveTextureVal->obj(), material->emissiveTexture))return false;
		}

		JSON::Value* emissiveFactorVal = (*materialObj)["emissiveFactor"];
		if (emissiveFactorVal)
		{
			if (!JSON::ArrayVector(emissiveFactorVal, material->emissiveFactor))return false;
		}

		std::string alphaModeStr;
		if (JSON::ApplyStrValue(materialObj, alphaModeStr, "alphaMode"))
		{
			bool alphaModeFound = false;
			for (auto const& alphaMode : AlphaModes)
			{
				if (alphaMode.second == alphaModeStr)
				{
					material->alphaMode = alphaMode.first;
					alphaModeFound = true;
					break;
				}
			}
			if (!alphaModeFound)return false;
		}

		JSON::ApplyNumValue(materialObj, material->alphaCutoff, "alphaCutoff");
		JSON::ApplyBoolValue(materialObj, material->doubleSided, "doubleSided");
	}

	return true;
}

bool glTF::ParsePbrMetallicRoughnessJson(JSON::Object* pbrObj, Material::PbrMetallicRoughness* pbr)
{
	if (!pbr) return false;

	JSON::Value* baseColorFactorVal = (*pbrObj)["baseColorFactor"];
	if (baseColorFactorVal)
	{
		if (!JSON::ArrayVector(baseColorFactorVal, pbr->baseColorFactor))return false;
	}

	JSON::Value* baseColorTextureVal = (*pbrObj)["baseColorTexture"];
	if (baseColorTextureVal)
	{
		if (!(baseColorTextureVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		if (pbr->baseColorTexture)return false;
		pbr->baseColorTexture = new Material::TextureInfo();
		if (!ParseTextureInfoJson(baseColorTextureVal->obj(), pbr->baseColorTexture))return false;
	}

	JSON::ApplyNumValue(pbrObj, pbr->metallicFactor, "metallicFactor");
	
	JSON::ApplyNumValue(pbrObj, pbr->roughnessFactor, "roughnessFactor");

	JSON::Value* metallicRoughnessTextureVal = (*pbrObj)["metallicRoughnessTexture"];
	if (metallicRoughnessTextureVal)
	{
		if (!(metallicRoughnessTextureVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		if (pbr->metallicRoughnessTexture)return false;
		pbr->metallicRoughnessTexture = new Material::TextureInfo();
		if (!ParseTextureInfoJson(metallicRoughnessTextureVal->obj(), pbr->metallicRoughnessTexture))return false;
	}

	return true;
}

bool glTF::ParseTextureInfoJson(JSON::Object* textureInfo, Material::TextureInfo* info)
{
	if (!info)return false;

	if (!JSON::ApplyIntValue(textureInfo, info->index, "index"))return false;
	JSON::ApplyIntValue(textureInfo, info->texCoord, "texCoord");

	return true;
}

bool glTF::ParseCamerasJson(JSON::Object* root, std::vector<Camera>& cameras)
{
	JSON::Value* camerasVal = (*root)["cameras"];
	if (!camerasVal)return true;

	if (!(camerasVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* camerasArr = camerasVal->arr();

	for (JSON::Value* cameraVal : camerasArr->value)
	{
		if (!(cameraVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* cameraObj = cameraVal->obj();

		cameras.push_back(Camera());
		Camera* camera = &cameras[cameras.size() - 1];

		std::string cameraTypeStr;
		if (!(JSON::ApplyStrValue(cameraObj, cameraTypeStr, "type")))return false;
		bool cameraTypeFound = false;
		for (auto const& cameraType : CameraTypes)
		{
			if (cameraType.second == cameraTypeStr)
			{
				camera->type = cameraType.first;
				cameraTypeFound = true;
				break;
			}
		}
		if (!cameraTypeFound)return false;
		JSON::ApplyStrValue(cameraObj, camera->name, "name");

		JSON::Value* orthographicVal = (*cameraObj)["orthographic"];
		if (orthographicVal)
		{
			if (!(orthographicVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* orthographicObj = orthographicVal->obj();

			if (camera->orthographic)return false;
			camera->orthographic = new Camera::Orthographic();

			if (!JSON::ApplyNumValue(orthographicObj, camera->orthographic->xmag, "xmag"))return false;
			if (!JSON::ApplyNumValue(orthographicObj, camera->orthographic->yamg, "yamg"))return false;
			if (!JSON::ApplyNumValue(orthographicObj, camera->orthographic->zfar, "zfar"))return false;
			if (!JSON::ApplyNumValue(orthographicObj, camera->orthographic->znear, "znear"))return false;
		}

		JSON::Value* perspectiveVal = (*cameraObj)["perspective"];
		if (perspectiveVal)
		{
			if (!(perspectiveVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* perspectiveObj = perspectiveVal->obj();

			if (camera->perspective)return false;
			camera->perspective = new Camera::Perspective();

			JSON::ApplyNumValue(perspectiveObj, camera->perspective->aspectRatio, "aspectRatio");
			if (!JSON::ApplyNumValue(perspectiveObj, camera->perspective->yfov, "yfov"))return false;
			JSON::ApplyNumValue(perspectiveObj, camera->perspective->zfar, "zfar");
			if (!JSON::ApplyNumValue(perspectiveObj, camera->perspective->znear, "znear"))return false;
		}
	}

	return true;
}

bool glTF::ParseSkinsJson(JSON::Object* root, std::vector<Skin>& skins)
{
	JSON::Value* skinsVal = (*root)["skins"];
	if (!skinsVal)return true;

	if (!(skinsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* skinsArr = skinsVal->arr();

	for (JSON::Value* skinVal : skinsArr->value)
	{
		if (!(skinVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* skinObj = skinVal->obj();

		Skin skin;

		JSON::Value* jointsVal = (*skinObj)["skins"];
		if (!jointsVal)return false;
		if (!(jointsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
		JSON::Array* jointsArr = jointsVal->arr();
		std::vector<int64> intArr = jointsArr->IntArr();
		for (auto el : intArr) skin.joints.push_back(static_cast<uint32>(el));

		JSON::ApplyStrValue(skinObj, skin.name, "name");

		JSON::ApplyIntValue(skinObj, skin.inverseBindMatrices, "inverseBindMatrices");
		JSON::ApplyIntValue(skinObj, skin.skeleton, "skeleton");

		skins.push_back(skin);
	}

	return true;
}

bool glTF::ParseAnimationsJson(JSON::Object* root, std::vector<Animation>& animations)
{
	JSON::Value* animationsVal = (*root)["animations"];
	if (!animationsVal)return true;

	if (!(animationsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* animationsArr = animationsVal->arr();

	for (JSON::Value* animationVal : animationsArr->value)
	{
		if (!(animationVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* animationObj = animationVal->obj();

		Animation animation;

		JSON::ApplyStrValue(animationObj, animation.name, "name");

		JSON::Value* channelsVal = (*animationObj)["channels"];
		if (!channelsVal)return false;
		if (!(channelsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
		JSON::Array* channelsArr = channelsVal->arr();
		for (JSON::Value* channelVal : channelsArr->value)
		{
			if (!(channelVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* channelObj = channelVal->obj();

			Animation::Channel channel;

			if (!JSON::ApplyIntValue(channelObj, channel.sampler, "sampler"))return false;

			JSON::Value* targetVal = (*channelObj)["target"];
			if (!targetVal)return false;
			if (!(targetVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* targetObj = targetVal->obj();

			JSON::ApplyIntValue(targetObj, channel.target.node, "node");

			std::string targetPathStr;
			if (!(JSON::ApplyStrValue(targetObj, targetPathStr, "path")))return false;
			bool targetPathFound = false;
			for (auto const& targetPath : ChannelTargetPaths)
			{
				if (targetPath.second == targetPathStr)
				{
					channel.target.path = targetPath.first;
					targetPathFound = true;
					break;
				}
			}
			if (!targetPathFound)return false;

			animation.channels.push_back(channel);
		}

		JSON::Value* samplersVal = (*animationObj)["samplers"];
		if (!samplersVal)return false;
		if (!(samplersVal->GetType() == JSON::ValueType::JSON_Array)) return false;
		JSON::Array* samplersArr = samplersVal->arr();
		for (JSON::Value* samplerVal : samplersArr->value)
		{
			if (!(samplerVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* samplerObj = samplerVal->obj();

			Animation::Sampler sampler;

			if (!JSON::ApplyIntValue(samplerObj, sampler.input, "input"))return false;
			if (!JSON::ApplyIntValue(samplerObj, sampler.output, "output"))return false;

			std::string samplerInterpolationStr;
			if (JSON::ApplyStrValue(samplerObj, samplerInterpolationStr, "interpolation"))
			{
				bool samplerInterpolationFound = false;
				for (auto const& samplerInterpolation : SamplerInterpolations)
				{
					if (samplerInterpolation.second == samplerInterpolationStr)
					{
						sampler.interpolation = samplerInterpolation.first;
						samplerInterpolationFound = true;
						break;
					}
				}
				if (!samplerInterpolationFound)return false;
			}

			animation.samplers.push_back(sampler);
		}

		animations.push_back(animation);
	}

	return true;
}

void glTF::LogGLTFVersionSupport()
{
	LOG(std::string("glTF minVersion ") + std::to_string(glTF::minVersion) + " maxVersion " + std::to_string(glTF::maxVersion));
}
