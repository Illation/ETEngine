#include "stdafx.h"
#include "GLTF.h"

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/BinaryReader.h>
#include <EtCore/IO/JsonParser.h>
#include <EtCore/IO/JsonDom.h>

#include <EtRendering/GraphicsTypes/Mesh.h>


namespace et {
namespace edit {


bool glTF::ParseGLTFData(const std::vector<uint8>& binaryContent, const std::string path, const std::string& extension, glTFAsset& asset)
{
	asset = glTFAsset();
	asset.basePath = path;

	std::string lowerExt;
	for (char val : extension)
	{
		if (val >= 'A' && val <= 'Z')
			lowerExt.push_back(val - ('A' - 'a'));
		else lowerExt.push_back(val);
	}
	if (lowerExt == "glb")
	{
		core::BinaryReader binReader;
		binReader.Open(binaryContent);
		if (!binReader.Exists())
		{
			LOG("glTF Failed to read the assetFile!", core::LogLevel::Warning);
			return false;
		}

		//Parse Header
		if (!ParseGLBHeader(&binReader, asset.header))
		{
			return false;
		}

		//Parse structured Json
		Chunk jsonChunk = Chunk();
		if (!ParseGLBChunk(&binReader, jsonChunk))
		{
			LOG("glTF failed to read json chunk from glb!", core::LogLevel::Warning);
			return false;
		}

		if (!(jsonChunk.chunkType == Chunk::ChunkType::JSON))
		{
			LOG("expected chunk type to be JSON", core::LogLevel::Warning);
			return false;
		}

		core::JSON::Parser parser = core::JSON::Parser(core::FileUtil::AsText(jsonChunk.chunkData));
		core::JSON::Object* root = parser.GetRoot();
		if (root == nullptr)return false;
		if (!glTF::ParseGlTFJson(root, asset.dom))return false;

		//Parse binary chunks
		while ((uint32)binReader.GetBufferPosition() < asset.header.length)
		{
			asset.dataChunks.push_back(Chunk());
			if (!ParseGLBChunk(&binReader, asset.dataChunks[asset.dataChunks.size() - 1]))
			{
				LOG("glTF failed to read binary chunk from glb!", core::LogLevel::Warning);
				return false;
			}

			if (!(asset.dataChunks[asset.dataChunks.size() - 1].chunkType == Chunk::ChunkType::BIN))
			{
				LOG("expected chunk type to be BIN", core::LogLevel::Warning);
				return false;
			}
		}

		return true;
	}
	else if (lowerExt == "gltf")
	{
		core::JSON::Parser parser = core::JSON::Parser(core::FileUtil::AsText(binaryContent));
		core::JSON::Object* root = parser.GetRoot();
		if (root == nullptr)return false;
		if (!glTF::ParseGlTFJson(root, asset.dom))return false;
		return true;
	}

	LOG("Unrecognized glTF extension", core::LogLevel::Warning);
	return false;
}

bool glTF::ParseGLBHeader(core::BinaryReader* binReader, Header &header)
{
	header.magic = binReader->Read<uint32>();
	if (!(header.magic == *reinterpret_cast<uint32*>("glTF")))
	{
		LOG("invalid glb file header!", core::LogLevel::Warning);
		return false;
	}
	header.version = binReader->Read<uint32>();
	if (!(header.version == 2))
	{
		LOG("invalid glb file header version!", core::LogLevel::Warning);
		return false;
	}
	header.length = binReader->Read<uint32>();
	return true;
}

bool glTF::ParseGLBChunk(core::BinaryReader* binReader, Chunk &chunk)
{
	if (binReader->GetBufferPosition() % 4u != 0u)//Make sure 4 byte alignment is respected
	{
		LOG("Expected binary buffer position for glb to be 4 byte aligned", core::LogLevel::Warning);
		binReader->SetBufferPosition(((binReader->GetBufferPosition() / 4u) + 1u) * 4u);
	}

	chunk.chunkLength = binReader->Read<uint32>();
	chunk.chunkType = static_cast<Chunk::ChunkType>(binReader->Read<uint32>());
	chunk.chunkData.reserve(chunk.chunkLength);
	for (uint32 i = 0; i < chunk.chunkLength; i++)
	{
		chunk.chunkData.push_back(binReader->Read<uint8>());
	}

	if (binReader->GetBufferPosition() % 4u != 0u)//Make sure 4 byte alignment is respected
	{
		binReader->SetBufferPosition(((binReader->GetBufferPosition() / 4u) + 1u) * 4u);
	}

	return true;
}

bool glTF::ParseGlTFJson(core::JSON::Object* json, Dom& dom)
{
	//Minimum requirement: asset and version
	if (!ParseAssetJson(json, dom.asset))
	{
		LOG("Failed to parse assset from JSON, invalid glTF file", core::LogLevel::Warning);
		return false;
	}
	//Also parse extensions and make sure we support them
	if (!ParseExtensionsJson(json, dom))
	{
		LOG("Failed to parse extensions from JSON, invalid glTF file", core::LogLevel::Warning);
		return false;
	}

	//Rest of the data we are interested in
	if (!ParseSceneJson(json, dom))
	{
		LOG("Failed to parse glTF scene from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseScenesJson(json, dom.scenes))
	{
		LOG("Failed to parse glTF scenes from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseNodesJson(json, dom.nodes))
	{
		LOG("Failed to parse glTF nodes from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseMeshesJson(json, dom.meshes))
	{
		LOG("Failed to parse glTF meshes from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseAccessorsJson(json, dom.accessors))
	{
		LOG("Failed to parse glTF accessors from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseBufferViewsJson(json, dom.bufferViews))
	{
		LOG("Failed to parse glTF bufferViews from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseBuffersJson(json, dom.buffers))
	{
		LOG("Failed to parse glTF buffers from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseImagesJson(json, dom.images))
	{
		LOG("Failed to parse glTF images from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseTexturesJson(json, dom.textures))
	{
		LOG("Failed to parse glTF textures from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseSamplersJson(json, dom.samplers))
	{
		LOG("Failed to parse glTF samplers from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseMaterialsJson(json, dom.materials))
	{
		LOG("Failed to parse glTF materials from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseCamerasJson(json, dom.cameras))
	{
		LOG("Failed to parse glTF cameras from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseSkinsJson(json, dom.skins))
	{
		LOG("Failed to parse glTF skins from JSON", core::LogLevel::Warning);
		return false;
	}
	if (!ParseAnimationsJson(json, dom.animations))
	{
		LOG("Failed to parse glTF animations from JSON", core::LogLevel::Warning);
		return false;
	}

	return true;
}

bool glTF::ParseAssetJson(core::JSON::Object* root, Asset& asset)
{
	core::JSON::Value* assetVal = (*root)["asset"];
	if (!assetVal)return false;
	if (!(assetVal->GetType() == core::JSON::ValueType::JSON_Object)) return false;
	core::JSON::Object* assetObj = assetVal->obj();

	if (!core::JSON::ApplyStrValue(assetObj, asset.version, "version")) return false;
	core::JSON::ApplyStrValue(assetObj, asset.minVersion, "minVersion");
	core::JSON::ApplyStrValue(assetObj, asset.generator, "generator");
	core::JSON::ApplyStrValue(assetObj, asset.copyright, "copyright");

	//Check version support
	bool hasMinVersion = false;
	if (asset.minVersion.size())
	{
		hasMinVersion = true;
		float minAssetVersion = std::stof(asset.minVersion);
		if (minAssetVersion < glTF::minVersion || minAssetVersion > glTF::maxVersion)
		{
			LOG("glTF minVersion is not supported by ETEngine", core::LogLevel::Warning);
			LogGLTFVersionSupport();
			return false;
		}
	}
	float version = std::stof(asset.version);
	if (version < glTF::minVersion)
	{
		LOG("glTF version is too low not supported by ETEngine", core::LogLevel::Warning);
		LogGLTFVersionSupport();
		return false;
	}
	if (!hasMinVersion)
	{
		if (version > glTF::maxVersion)
		{
			LOG("glTF version is too high not supported by ETEngine", core::LogLevel::Warning);
			LogGLTFVersionSupport();
			return false;
		}
	}

	return true;
}

bool glTF::ParseExtensionsJson(core::JSON::Object* root, Dom& dom)
{
	core::JSON::Value* extUsedVal = (*root)["extensionsRequired"];
	if (extUsedVal)
	{
		if (!(extUsedVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
		dom.extensionsUsed = extUsedVal->arr()->StrArr();
	}

	core::JSON::Value* extRequVal = (*root)["extensionsRequired"];
	if (extRequVal)
	{
		if (!(extRequVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
		dom.extensionsRequired = extRequVal->arr()->StrArr();
	}

	//Check extension support
	for (auto requ : dom.extensionsRequired)
	{
		if (std::find(dom.extensionsUsed.begin(), dom.extensionsUsed.end(), requ) == dom.extensionsUsed.end())
		{
			LOG(std::string("Required glTF extension '") + requ + "' not found in used extensions", core::LogLevel::Warning);
			return false;
		}
		if (std::find(supportedExtensions.begin(), supportedExtensions.end(), requ) == supportedExtensions.end())
		{
			LOG(std::string("Required glTF extension '") + requ + "' is not supported by ETEngine", core::LogLevel::Warning);
			return false;
		}
	}
	return true;
}

bool glTF::ParseSceneJson(core::JSON::Object* root, Dom& dom)
{
	core::JSON::ApplyIntValue(root, dom.scene, "scene");
	return true;
}

bool glTF::ParseScenesJson(core::JSON::Object* root, std::vector<Scene>& scenes)
{
	core::JSON::Value* scenesVal = (*root)["scenes"];
	if (!scenesVal)return true;

	if (!(scenesVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
	core::JSON::Array* scenesArr = scenesVal->arr();

	for (core::JSON::Value* sceneVal : scenesArr->value)
	{
		if (!(sceneVal->GetType() == core::JSON::ValueType::JSON_Object)) return false;
		core::JSON::Object* sceneObj = sceneVal->obj();

		Scene scene;

		core::JSON::ApplyStrValue(sceneObj, scene.name, "name");

		core::JSON::Value* nodesVal = (*sceneObj)["nodes"];
		if (nodesVal)
		{
			if (!(nodesVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
			core::JSON::Array* nodesArr = nodesVal->arr();
			std::vector<int64> intArr = nodesArr->IntArr();
			for (auto el : intArr) scene.nodes.push_back(static_cast<uint32>(el));
		}

		scenes.push_back(scene);
	}

	return true;
}

bool glTF::ParseNodesJson(core::JSON::Object* root, std::vector<Node>& nodes)
{
	core::JSON::Value* nodesVal = (*root)["nodes"];
	if (!nodesVal)return true;

	if (!(nodesVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
	core::JSON::Array* nodesArr = nodesVal->arr();

	for (core::JSON::Value* nodeVal : nodesArr->value)
	{
		if (!(nodeVal->GetType() == core::JSON::ValueType::JSON_Object)) return false;
		core::JSON::Object* nodeObj = nodeVal->obj();

		Node node;

		core::JSON::ApplyStrValue(nodeObj, node.name, "name");

		core::JSON::Value* childrenVal = (*nodeObj)["children"];
		if (childrenVal)
		{
			if (!(childrenVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
			core::JSON::Array* childrenArr = childrenVal->arr();
			std::vector<int64> intArr = childrenArr->IntArr();
			for (auto el : intArr) node.children.push_back(static_cast<uint32>(el));
		}

		bool hasTranslation = false;
		core::JSON::Value* translationVal = (*nodeObj)["translation"];
		if (translationVal)
		{
			if (!core::JSON::ArrayVector(translationVal, node.translation))return false;
			hasTranslation = true;
		}

		bool hasRotation = false;
		core::JSON::Value* rotationVal = (*nodeObj)["rotation"];
		if (rotationVal)
		{
			if (!core::JSON::ArrayVector(rotationVal, node.rotation.v4))return false;
			hasRotation = true;
		}

		bool hasScale = false;
		core::JSON::Value* scaleVal = (*nodeObj)["scale"];
		if (scaleVal)
		{
			if (!core::JSON::ArrayVector(scaleVal, node.scale))return false;
			hasScale = true;
		}

		bool hasMatrix = false;
		core::JSON::Value* matrixVal = (*nodeObj)["matrix"];
		if (matrixVal)
		{
			if (!core::JSON::ArrayMatrix(matrixVal, node.matrix))return false;
			hasMatrix = true;
		}

		if (hasMatrix)
		{
			vec3 trans, scale;
			quat rot;
			math::decomposeTRS(node.matrix, trans, rot, scale);
			if (!hasTranslation)node.translation = trans;
			else if (!math::nearEqualsV(node.translation, trans, 0.0001f))LOG("inconsistent translation values for node", core::LogLevel::Warning);
			if (!hasRotation)node.rotation = rot;
			else
			{
				//different quaternions can express the same rotation
				vec3 testVec = vec3(0, 0, 1);
				if (!math::nearEqualsV(node.rotation * testVec, rot * testVec, 0.0001f))LOG("inconsistent rotation values for node", core::LogLevel::Warning);
			}
			if (!hasScale)node.scale = scale;
			else if (!math::nearEqualsV(node.scale, scale, 0.0001f))LOG("inconsistent scale values for node", core::LogLevel::Warning);
		}
		else
		{
			node.matrix = math::scale(node.scale) * math::rotate(node.rotation) * math::translate(node.translation);
		}

		core::JSON::ApplyIntValue(nodeObj, node.camera, "camera");
		core::JSON::ApplyIntValue(nodeObj, node.mesh, "mesh");
		core::JSON::ApplyIntValue(nodeObj, node.skin, "skin");

		core::JSON::Value* weightsVal = (*nodeObj)["weights"];
		if (weightsVal)
		{
			if (!(weightsVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
			core::JSON::Array* weightsArr = weightsVal->arr();
			std::vector<double> arr = weightsArr->NumArr();
			for (auto el : arr) node.weights.push_back(static_cast<float>(el));
		}

		nodes.push_back(node);
	}

	return true;
}

bool glTF::ParseMeshesJson(core::JSON::Object* root, std::vector<Mesh>& meshes)
{
	core::JSON::Value* meshesVal = (*root)["meshes"];
	if (!meshesVal)return true;

	if (!(meshesVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
	core::JSON::Array* meshesArr = meshesVal->arr();

	for (core::JSON::Value* meshVal : meshesArr->value)
	{
		if (!(meshVal->GetType() == core::JSON::ValueType::JSON_Object)) return false;
		core::JSON::Object* meshObj = meshVal->obj();

		Mesh mesh;

		core::JSON::ApplyStrValue(meshObj, mesh.name, "name");

		core::JSON::Value* primitivesVal = (*meshObj)["primitives"];
		if (primitivesVal)
		{
			if (!(primitivesVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
			core::JSON::Array* primitivesArr = primitivesVal->arr();
			for (core::JSON::Value* primitiveVal : primitivesArr->value)
			{
				if (!(primitiveVal->GetType() == core::JSON::ValueType::JSON_Object)) return false;
				core::JSON::Object* primitiveObj = primitiveVal->obj();
				Primitive prim;
				if (ParsePrimitiveJson(primitiveObj, prim))
				{
					mesh.primitives.push_back(prim);
				}
				else return false;
			}
		}

		core::JSON::Value* weightsVal = (*meshObj)["weights"];
		if (weightsVal)
		{
			if (!(weightsVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
			core::JSON::Array* nodesArr = weightsVal->arr();
			std::vector<double> weightArr = nodesArr->NumArr();
			for (auto el : weightArr) mesh.weights.push_back(static_cast<float>(el));
		}

		meshes.push_back(mesh);
	}

	return true;
}

bool glTF::ParsePrimitiveJson(core::JSON::Object* primitiveObj, Primitive& primitive)
{
	core::JSON::Value* attributesVal = (*primitiveObj)["attributes"];
	if (!attributesVal)return false;
	if (!(attributesVal->GetType() == core::JSON::ValueType::JSON_Object)) return false;
	core::JSON::Object* attributeObj = attributesVal->obj();
	core::JSON::ApplyIntValue(attributeObj, primitive.attributes.position, "POSITION");
	core::JSON::ApplyIntValue(attributeObj, primitive.attributes.normal, "NORMAL");
	core::JSON::ApplyIntValue(attributeObj, primitive.attributes.tangent, "TANGENT");
	core::JSON::ApplyIntValue(attributeObj, primitive.attributes.texcoord0, "TEXCOORD_0");
	core::JSON::ApplyIntValue(attributeObj, primitive.attributes.texcoord1, "TEXCOORD_1");
	core::JSON::ApplyIntValue(attributeObj, primitive.attributes.color0, "COLOR_0");
	core::JSON::ApplyIntValue(attributeObj, primitive.attributes.joints0, "JOINTS_0");
	core::JSON::ApplyIntValue(attributeObj, primitive.attributes.weights0, "WEIGHTS_0");

	core::JSON::ApplyIntValue(primitiveObj, primitive.indices, "indices");
	core::JSON::ApplyIntValue(primitiveObj, primitive.material, "material");
	core::JSON::ApplyIntValue(primitiveObj, primitive.mode, "mode");

	core::JSON::Value* targetsVal = (*primitiveObj)["targets"];
	if (targetsVal)
	{
		if (!(targetsVal->GetType() == core::JSON::ValueType::JSON_Array)) return false;
		core::JSON::Array* targetsArr = targetsVal->arr();
		for (core::JSON::Value* targetVal : targetsArr->value)
		{
			if (!(targetVal->GetType() == core::JSON::ValueType::JSON_Object)) return false;
			core::JSON::Object* targetObj = targetVal->obj();

			Primitive::Targets target;

			core::JSON::ApplyIntValue(targetObj, target.position, "POSITION");
			core::JSON::ApplyIntValue(targetObj, target.normal, "NORMAL");
			core::JSON::ApplyIntValue(targetObj, target.tangent, "TANGENT");

			primitive.targets.push_back(target);
		}
	}

	return true;
}

bool glTF::ParseAccessorsJson(core::JSON::Object* root, std::vector<Accessor>& accessors)
{
	using namespace core;

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

			accessors[accessors.size() - 1].sparse = new Accessor::Sparse();
			Accessor::Sparse* sparse = accessors[accessors.size() - 1].sparse;

			if (!JSON::ApplyIntValue(sparseObj, sparse->count, "count"))return false;

			JSON::Value* indicesVal = (*sparseObj)["indices"];
			if (!indicesVal)return false;
			if (!(indicesVal->GetType() == JSON::ValueType::JSON_Object)) return false;
			JSON::Object* indicesObj = indicesVal->obj();
			if (!JSON::ApplyIntValue(indicesObj, sparse->indices.bufferView, "bufferView"))return false;
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

bool glTF::ParseBufferViewsJson(core::JSON::Object* root, std::vector<BufferView>& bufferViews)
{
	using namespace core;

	JSON::Value* bufferViewsVal = (*root)["bufferViews"];
	if (!bufferViewsVal)return true;

	if (!(bufferViewsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* bufferViewsArr = bufferViewsVal->arr();

	for (JSON::Value* bufferViewVal : bufferViewsArr->value)
	{
		if (!(bufferViewVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* bufferViewObj = bufferViewVal->obj();

		BufferView view;

		if (!JSON::ApplyIntValue(bufferViewObj, view.buffer, "buffer"))return false;
		if (!JSON::ApplyIntValue(bufferViewObj, view.byteLength, "byteLength"))return false;
		JSON::ApplyIntValue(bufferViewObj, view.byteOffset, "byteOffset");
		JSON::ApplyIntValue(bufferViewObj, view.byteStride, "byteStride");
		JSON::ApplyIntValue(bufferViewObj, view.target, "target");
		JSON::ApplyStrValue(bufferViewObj, view.name, "name");

		bufferViews.push_back(view);
	}

	return true;
}

bool glTF::ParseBuffersJson(core::JSON::Object* root, std::vector<Buffer>& buffers)
{
	using namespace core;

	JSON::Value* buffersVal = (*root)["buffers"];
	if (!buffersVal)return true;

	if (!(buffersVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* buffersArr = buffersVal->arr();

	for (JSON::Value* bufferVal : buffersArr->value)
	{
		if (!(bufferVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* bufferObj = bufferVal->obj();

		Buffer buffer;

		JSON::Value const* const jval = (*bufferObj)["uri"];
		if (jval)
		{
			JSON::String const* const jstr = jval->str();
			if (jstr)
			{
				buffer.uri.SetPath(jstr->value);
			}
		}

		if (!JSON::ApplyIntValue(bufferObj, buffer.byteLength, "byteLength"))
		{
			return false;
		}

		JSON::ApplyStrValue(bufferObj, buffer.name, "name");

		buffers.push_back(buffer);
	}

	return true;
}

bool glTF::ParseTexturesJson(core::JSON::Object* root, std::vector<Texture>& textures)
{
	using namespace core;

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

bool glTF::ParseImagesJson(core::JSON::Object* root, std::vector<Image>& images)
{
	using namespace core;

	JSON::Value* imagesVal = (*root)["images"];
	if (!imagesVal)return true;

	if (!(imagesVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* imagesArr = imagesVal->arr();

	for (JSON::Value* imageVal : imagesArr->value)
	{
		if (!(imageVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* imageObj = imageVal->obj();

		Image image;

		JSON::Value const* const jval = (*imageObj)["uri"];
		if (jval)
		{
			JSON::String const* const jstr = jval->str();
			if (jstr)
			{
				image.uri.SetPath(jstr->value);
			}
		}

		JSON::ApplyIntValue(imageObj, image.bufferView, "bufferView");
		JSON::ApplyStrValue(imageObj, image.mimeType, "mimeType");
		JSON::ApplyStrValue(imageObj, image.name, "name");

		images.push_back(image);
	}

	return true;
}

bool glTF::ParseSamplersJson(core::JSON::Object* root, std::vector<Sampler>& samplers)
{
	using namespace core;

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

bool glTF::ParseMaterialsJson(core::JSON::Object* root, std::vector<Material>& materials)
{
	using namespace core;

	JSON::Value* materialsVal = (*root)["materials"];
	if (!materialsVal)return true;

	if (!(materialsVal->GetType() == JSON::ValueType::JSON_Array)) return false;
	JSON::Array* materialsArr = materialsVal->arr();

	for (JSON::Value* materialVal : materialsArr->value)
	{
		if (!(materialVal->GetType() == JSON::ValueType::JSON_Object)) return false;
		JSON::Object* materialObj = materialVal->obj();

		materials.push_back(Material());
		Material* material = &materials[materials.size() - 1];

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

bool glTF::ParsePbrMetallicRoughnessJson(core::JSON::Object* pbrObj, Material::PbrMetallicRoughness* pbr)
{
	using namespace core;

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

bool glTF::ParseTextureInfoJson(core::JSON::Object* textureInfo, Material::TextureInfo* info)
{
	using namespace core;

	if (!info)return false;

	if (!JSON::ApplyIntValue(textureInfo, info->index, "index"))return false;
	JSON::ApplyIntValue(textureInfo, info->texCoord, "texCoord");

	return true;
}

bool glTF::ParseCamerasJson(core::JSON::Object* root, std::vector<Camera>& cameras)
{
	using namespace core;

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

bool glTF::ParseSkinsJson(core::JSON::Object* root, std::vector<Skin>& skins)
{
	using namespace core;

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

bool glTF::ParseAnimationsJson(core::JSON::Object* root, std::vector<Animation>& animations)
{
	using namespace core;

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

bool glTF::OpenBufferViewReader(glTFAsset& asset, uint32 viewIdx, core::BinaryReader* pViewReader)
{
	if (viewIdx >= (int32)asset.dom.bufferViews.size())
	{
		LOG("BufferView index out of range", core::LogLevel::Warning);
		return false;
	}

	BufferView& view = asset.dom.bufferViews[viewIdx];
	if (view.buffer >= (uint32)asset.dom.buffers.size())
	{
		LOG("Buffer index out of range", core::LogLevel::Warning);
		return false;
	}

	Buffer& buffer = asset.dom.buffers[view.buffer];
	if (!buffer.uri.IsEvaluated())
	{
		if (!buffer.uri.Evaluate(asset.basePath))
		{
			LOG("Failed to evaluate buffer URI", core::LogLevel::Warning);
			return false;
		}
	}

	if (buffer.uri.GetType() == core::URI::E_Type::None)
	{
		if (view.buffer >= static_cast<uint32>(asset.dataChunks.size()))
		{
			LOG("No data chunk loaded for glb buffer", core::LogLevel::Warning);
			return false;
		}

		pViewReader->Open(asset.dataChunks[view.buffer].chunkData, static_cast<size_t>(view.byteOffset), static_cast<size_t>(view.byteLength));
	}
	else
	{
		pViewReader->Open(buffer.uri.GetEvaluatedData(), static_cast<size_t>(view.byteOffset), static_cast<size_t>(view.byteLength));
	}

	if (!pViewReader->Exists())
	{
		LOG("glTF Failed to read the buffer view!", core::LogLevel::Warning);
		return false;
	}

	return true;
}

bool glTF::GetAccessorData(glTFAsset& asset, uint32 idx, std::vector<uint8>& data)
{
	if (idx >= (uint32)asset.dom.accessors.size())
	{
		LOG("Accessor index out of range", core::LogLevel::Warning);
		return false;
	}
	Accessor& accessor = asset.dom.accessors[idx];
	if (accessor.sparse || accessor.bufferView == -1)
	{
		LOG("Unsupported accessor type, sparse accessors are not yet implemented", core::LogLevel::Warning);
		return false;
	}
	if (accessor.bufferView >= (int32)asset.dom.bufferViews.size())
	{
		LOG("BufferView index out of range", core::LogLevel::Warning);
		return false;
	}
	BufferView& view = asset.dom.bufferViews[accessor.bufferView];
	if (view.buffer >= (uint32)asset.dom.buffers.size())
	{
		LOG("Buffer index out of range", core::LogLevel::Warning);
		return false;
	}

	uint8 compSize = ComponentTypes[accessor.componentType];
	uint8 compsPerEl = AccessorTypes[accessor.type].first;
	uint8 elSize = compSize * compsPerEl;

	//Validation
	if (!(accessor.byteOffset % compSize == 0)) LOG("Accessors byte offset needs to be a multiple of the component size", core::LogLevel::Warning);
	if (accessor.min.size())
	{
		if (!((uint32)accessor.min.size() == (uint32)compsPerEl)) LOG("Accessors min array size must equal components per element", core::LogLevel::Warning);
	}
	if (accessor.max.size())
	{
		if (!((uint32)accessor.max.size() == (uint32)compsPerEl)) LOG("Accessors max array size must equal components per element", core::LogLevel::Warning);
	}

	uint32 stride = (view.byteStride == -1) ? (uint32)elSize : view.byteStride;
	if (!(stride % compSize == 0))
	{
		LOG("Accessors byte stride needs to be a multiple of the component size", core::LogLevel::Warning);
	}

	if ((accessor.byteOffset + stride * (accessor.count - 1) + elSize) > view.byteLength)
	{
		LOG("Accessors doesn't fit buffer view", core::LogLevel::Warning);
	}

	core::BinaryReader* pViewReader = new core::BinaryReader();
	if (!OpenBufferViewReader(asset, accessor.bufferView, pViewReader))
	{
		delete pViewReader;
		LOG("Unable to read buffer view", core::LogLevel::Warning);
		return false;
	}

	for (uint64 i = static_cast<uint64>(accessor.byteOffset);
		i < static_cast<uint64>(accessor.byteOffset) + (stride * accessor.count);
		i += static_cast<uint64>(stride))
	{
		pViewReader->SetBufferPosition(static_cast<size_t>(i));
		for (uint32 j = 0; j < elSize; ++j)
		{
			data.push_back(pViewReader->Read<uint8>());
		}
	}

	delete pViewReader;
	return true;
}


} // namespace edit
} // namespace et
