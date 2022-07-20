#include <EtEditor/stdafx.h>
#include "GltfParser.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/BinaryReader.h>
#include <EtCore/IO/JsonParser.h>
#include <EtCore/IO/JsonDom.h>


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
		{
			lowerExt.push_back(val - ('A' - 'a'));
		}
		else 
		{
			lowerExt.push_back(val);
		}
	}

	if (lowerExt == "glb")
	{
		core::BinaryReader binReader;
		binReader.Open(binaryContent);
		if (!binReader.Exists())
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF Failed to read the assetFile!");
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
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read json chunk from glb!");
			return false;
		}

		if (!(jsonChunk.chunkType == Chunk::ChunkType::JSON))
		{
			ET_LOG_E(ET_CTX_EDITOR, "expected chunk type to be JSON");
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
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read binary chunk from glb!");
				return false;
			}

			if (!(asset.dataChunks[asset.dataChunks.size() - 1].chunkType == Chunk::ChunkType::BIN))
			{
				ET_LOG_E(ET_CTX_EDITOR, "expected chunk type to be BIN");
				return false;
			}
		}

		return true;
	}
	else if (lowerExt == "gltf")
	{
		core::JSON::Parser parser = core::JSON::Parser(core::FileUtil::AsText(binaryContent));
		core::JSON::Object* root = parser.GetRoot();
		if (root == nullptr)
		{
			ET_LOG_E(ET_CTX_EDITOR, "failed to read glTF as JSON document");
			return false;
		}

		if (!glTF::ParseGlTFJson(root, asset.dom))
		{
			ET_LOG_E(ET_CTX_EDITOR, "failed to parse JSON into glTF DOM");
			return false;
		}

		return true;
	}

	ET_LOG_E(ET_CTX_EDITOR, "Unrecognized glTF extension");
	return false;
}

bool glTF::ParseGLBHeader(core::BinaryReader* binReader, Header &header)
{
	header.magic = binReader->Read<uint32>();
	if (!(header.magic == *reinterpret_cast<uint32*>("glTF")))
	{
		ET_LOG_E(ET_CTX_EDITOR, "invalid glb file header!");
		return false;
	}

	header.version = binReader->Read<uint32>();
	if (!(header.version == 2))
	{
		ET_LOG_E(ET_CTX_EDITOR, "invalid glb file header version!");
		return false;
	}

	header.length = binReader->Read<uint32>();
	return true;
}

bool glTF::ParseGLBChunk(core::BinaryReader* binReader, Chunk &chunk)
{
	if (binReader->GetBufferPosition() % 4u != 0u)//Make sure 4 byte alignment is respected
	{
		ET_LOG_W(ET_CTX_EDITOR, "Expected binary buffer position for glb to be 4 byte aligned");
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
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse assset from JSON, invalid glTF file");
		return false;
	}

	//Also parse extensions and make sure we support them
	if (!ParseExtensionsJson(json, dom))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse extensions from JSON, invalid glTF file");
		return false;
	}

	//Rest of the data we are interested in
	if (!ParseSceneJson(json, dom))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF scene from JSON");
		return false;
	}

	if (!ParseScenesJson(json, dom.scenes))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF scenes from JSON");
		return false;
	}

	if (!ParseNodesJson(json, dom.nodes))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF nodes from JSON");
		return false;
	}

	if (!ParseMeshesJson(json, dom.meshes))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF meshes from JSON");
		return false;
	}

	if (!ParseAccessorsJson(json, dom.accessors))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF accessors from JSON");
		return false;
	}

	if (!ParseBufferViewsJson(json, dom.bufferViews))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF bufferViews from JSON");
		return false;
	}

	if (!ParseBuffersJson(json, dom.buffers))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF buffers from JSON");
		return false;
	}

	if (!ParseImagesJson(json, dom.images))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF images from JSON");
		return false;
	}

	if (!ParseTexturesJson(json, dom.textures))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF textures from JSON");
		return false;
	}

	if (!ParseSamplersJson(json, dom.samplers))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF samplers from JSON");
		return false;
	}

	if (!ParseMaterialsJson(json, dom.materials))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF materials from JSON");
		return false;
	}

	if (!ParseCamerasJson(json, dom.cameras))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF cameras from JSON");
		return false;
	}

	if (!ParseSkinsJson(json, dom.skins))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF skins from JSON");
		return false;
	}

	if (!ParseAnimationsJson(json, dom.animations))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Failed to parse glTF animations from JSON");
		return false;
	}

	ET_LOG_I(ET_CTX_EDITOR, "glTF DOM parsed");
	return true;
}

bool glTF::ParseAssetJson(core::JSON::Object* root, Asset& asset)
{
	core::JSON::Value* assetVal = (*root)["asset"];
	if (assetVal == nullptr)
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF doesn't have an asset");
		return false;
	}

	if (!(assetVal->GetType() == core::JSON::ValueType::JSON_Object))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF asset is not a JSON object");
		return false;
	}

	core::JSON::Object* assetObj = assetVal->obj();

	if (!core::JSON::ApplyStrValue(assetObj, asset.version, "version"))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF asset missing version");
		return false;
	}

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
			ET_LOG_E(ET_CTX_EDITOR, "glTF minVersion is not supported by ETEngine");
			LogGLTFVersionSupport();
			return false;
		}
	}

	float version = std::stof(asset.version);
	if (version < glTF::minVersion)
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF version is too low not supported by ETEngine");
		LogGLTFVersionSupport();
		return false;
	}

	if (!hasMinVersion)
	{
		if (version > glTF::maxVersion)
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF version is too high not supported by ETEngine");
			LogGLTFVersionSupport();
			return false;
		}
	}

	return true;
}

bool glTF::ParseExtensionsJson(core::JSON::Object* root, Dom& dom)
{
	core::JSON::Value* extUsedVal = (*root)["extensionsRequired"];
	if (extUsedVal != nullptr)
	{
		if (!(extUsedVal->GetType() == core::JSON::ValueType::JSON_Array))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'extensionsUsed' to be a JSON array");
			return false;
		}

		dom.extensionsUsed = extUsedVal->arr()->StrArr();
	}

	core::JSON::Value* extRequVal = (*root)["extensionsRequired"];
	if (extRequVal != nullptr)
	{
		if (!(extRequVal->GetType() == core::JSON::ValueType::JSON_Array))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'extensionsRequired' to be a JSON array");
			return false;
		}

		dom.extensionsRequired = extRequVal->arr()->StrArr();
	}

	//Check extension support
	for (std::string const& requ : dom.extensionsRequired)
	{
		if (std::find(dom.extensionsUsed.begin(), dom.extensionsUsed.end(), requ) == dom.extensionsUsed.end())
		{
			ET_LOG_E(ET_CTX_EDITOR, "Required glTF extension '%s' not found in used extensions", requ.c_str());
			return false;
		}

		if (std::find(supportedExtensions.begin(), supportedExtensions.end(), requ) == supportedExtensions.end())
		{
			ET_LOG_E(ET_CTX_EDITOR, "Required glTF extension '%s' is not supported by ETEngine", requ.c_str());
			return false;
		}
	}

	ET_LOG_V(ET_CTX_EDITOR, "All glTF extensions supported");
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
	if (scenesVal == nullptr)
	{
		return true;
	}

	if (!(scenesVal->GetType() == core::JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'scenes' to be a JSON array");
		return false;
	}

	core::JSON::Array* scenesArr = scenesVal->arr();

	for (core::JSON::Value* sceneVal : scenesArr->value)
	{
		if (!(sceneVal->GetType() == core::JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'scenes' element to be a JSON object");
			return false;
		}

		core::JSON::Object* sceneObj = sceneVal->obj();

		Scene scene;

		core::JSON::ApplyStrValue(sceneObj, scene.name, "name");

		core::JSON::Value* nodesVal = (*sceneObj)["nodes"];
		if (nodesVal)
		{
			if (!(nodesVal->GetType() == core::JSON::ValueType::JSON_Array))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected scene 'nodes' to be a JSON array");
				return false;
			}

			core::JSON::Array* nodesArr = nodesVal->arr();
			std::vector<int64> intArr = nodesArr->IntArr();
			for (auto el : intArr)
			{
				scene.nodes.push_back(static_cast<uint32>(el));
			}
		}

		scenes.push_back(scene);
	}

	return true;
}

bool glTF::ParseNodesJson(core::JSON::Object* root, std::vector<Node>& nodes)
{
	core::JSON::Value* nodesVal = (*root)["nodes"];
	if (nodesVal == nullptr)
	{
		return true;
	}

	if (!(nodesVal->GetType() == core::JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'nodes' to be a JSON array");
		return false;
	}

	core::JSON::Array* nodesArr = nodesVal->arr();

	for (core::JSON::Value* nodeVal : nodesArr->value)
	{
		if (!(nodeVal->GetType() == core::JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'nodes' element to be a JSON object");
			return false;
		}

		core::JSON::Object* nodeObj = nodeVal->obj();

		Node node;

		core::JSON::ApplyStrValue(nodeObj, node.name, "name");

		core::JSON::Value* childrenVal = (*nodeObj)["children"];
		if (childrenVal != nullptr)
		{
			if (!(childrenVal->GetType() == core::JSON::ValueType::JSON_Array))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected node 'children' to be a JSON array");
				return false;
			}

			core::JSON::Array* childrenArr = childrenVal->arr();
			std::vector<int64> intArr = childrenArr->IntArr();
			for (auto el : intArr)
			{
				node.children.push_back(static_cast<uint32>(el));
			}
		}

		bool hasTranslation = false;
		core::JSON::Value* translationVal = (*nodeObj)["translation"];
		if (translationVal != nullptr)
		{
			if (!core::JSON::ArrayVector(translationVal, node.translation))
			{
				ET_LOG_E(ET_CTX_EDITOR, "failed to apply node array 'translation' to a vector");
				return false;
			}

			hasTranslation = true;
		}

		bool hasRotation = false;
		core::JSON::Value* rotationVal = (*nodeObj)["rotation"];
		if (rotationVal != nullptr)
		{
			if (!core::JSON::ArrayVector(rotationVal, node.rotation.v4))
			{
				ET_LOG_E(ET_CTX_EDITOR, "failed to apply node array 'rotation' to a vector");
				return false;
			}

			hasRotation = true;
		}

		bool hasScale = false;
		core::JSON::Value* scaleVal = (*nodeObj)["scale"];
		if (scaleVal != nullptr)
		{
			if (!core::JSON::ArrayVector(scaleVal, node.scale))
			{
				ET_LOG_E(ET_CTX_EDITOR, "failed to apply node array 'scale' to a vector");
				return false;
			}

			hasScale = true;
		}

		bool hasMatrix = false;
		core::JSON::Value* matrixVal = (*nodeObj)["matrix"];
		if (matrixVal != nullptr)
		{
			if (!core::JSON::ArrayMatrix(matrixVal, node.matrix))
			{
				ET_LOG_E(ET_CTX_EDITOR, "failed to apply node array 'matrix' to a matrix");
				return false;
			}

			hasMatrix = true;
		}

		if (hasMatrix)
		{
			vec3 trans, scale;
			quat rot;
			math::decomposeTRS(node.matrix, trans, rot, scale);
			if (!hasTranslation)
			{
				node.translation = trans;
			}
			else if (!math::nearEqualsV(node.translation, trans, 0.0001f))
			{
				ET_LOG_W(ET_CTX_EDITOR, "inconsistent translation values for node");
			}

			if (!hasRotation)
			{
				node.rotation = rot;
			}
			else
			{
				//different quaternions can express the same rotation
				vec3 testVec = vec3(0, 0, 1);
				if (!math::nearEqualsV(node.rotation * testVec, rot * testVec, 0.0001f))
				{
					ET_LOG_W(ET_CTX_EDITOR, "inconsistent rotation values for node");
				}
			}

			if (!hasScale)
			{
				node.scale = scale;
			}
			else if (!math::nearEqualsV(node.scale, scale, 0.0001f))
			{
				ET_LOG_W(ET_CTX_EDITOR, "inconsistent scale values for node");
			}
		}
		else
		{
			node.matrix = math::scale(node.scale) * math::rotate(node.rotation) * math::translate(node.translation);
		}

		core::JSON::ApplyIntValue(nodeObj, node.camera, "camera");
		core::JSON::ApplyIntValue(nodeObj, node.mesh, "mesh");
		core::JSON::ApplyIntValue(nodeObj, node.skin, "skin");

		core::JSON::Value* weightsVal = (*nodeObj)["weights"];
		if (weightsVal != nullptr)
		{
			if (!(weightsVal->GetType() == core::JSON::ValueType::JSON_Array))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected node 'weights' to be a JSON array");
				return false;
			}

			core::JSON::Array* weightsArr = weightsVal->arr();
			std::vector<double> arr = weightsArr->NumArr();
			for (auto el : arr)
			{
				node.weights.push_back(static_cast<float>(el));
			}
		}

		nodes.push_back(node);
	}

	return true;
}

bool glTF::ParseMeshesJson(core::JSON::Object* root, std::vector<Mesh>& meshes)
{
	core::JSON::Value* meshesVal = (*root)["meshes"];
	if (meshesVal == nullptr)
	{
		return true;
	}

	if (!(meshesVal->GetType() == core::JSON::ValueType::JSON_Array)) 
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'meshes' to be a JSON array");
		return false;
	}

	core::JSON::Array* meshesArr = meshesVal->arr();

	for (core::JSON::Value* meshVal : meshesArr->value)
	{
		if (!(meshVal->GetType() == core::JSON::ValueType::JSON_Object)) 
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'meshes' element to be a JSON object");
			return false;
		}

		core::JSON::Object* meshObj = meshVal->obj();

		Mesh mesh;

		core::JSON::ApplyStrValue(meshObj, mesh.name, "name");

		core::JSON::Value* primitivesVal = (*meshObj)["primitives"];
		if (primitivesVal != nullptr)
		{
			if (!(primitivesVal->GetType() == core::JSON::ValueType::JSON_Array)) 
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected mesh 'primitives' to be a JSON array");
				return false;
			}

			core::JSON::Array* primitivesArr = primitivesVal->arr();
			for (core::JSON::Value* primitiveVal : primitivesArr->value)
			{
				if (!(primitiveVal->GetType() == core::JSON::ValueType::JSON_Object)) 
				{
					ET_LOG_E(ET_CTX_EDITOR, "glTF expected mesh 'primitives' element to be a JSON object");
					return false;
				}

				core::JSON::Object* primitiveObj = primitiveVal->obj();
				Primitive prim;
				if (ParsePrimitiveJson(primitiveObj, prim))
				{
					mesh.primitives.push_back(prim);
				}
				else 
				{
					ET_LOG_E(ET_CTX_EDITOR, "glTF failed to parse mesh primitive");
					return false;
				}
			}
		}

		core::JSON::Value* weightsVal = (*meshObj)["weights"];
		if (weightsVal != nullptr)
		{
			if (!(weightsVal->GetType() == core::JSON::ValueType::JSON_Array)) 
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected mesh 'weights' to be a JSON array");
				return false;
			}

			core::JSON::Array* nodesArr = weightsVal->arr();
			std::vector<double> weightArr = nodesArr->NumArr();
			for (auto el : weightArr) 
			{
				mesh.weights.push_back(static_cast<float>(el));
			}
		}

		meshes.push_back(mesh);
	}

	return true;
}

bool glTF::ParsePrimitiveJson(core::JSON::Object* primitiveObj, Primitive& primitive)
{
	core::JSON::Value* attributesVal = (*primitiveObj)["attributes"];
	if (!attributesVal)
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected primitive to have an 'attributes' element");
		return false;
	}

	if (!(attributesVal->GetType() == core::JSON::ValueType::JSON_Object)) 
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected primitive 'attributes' to be a JSON object");
		return false;
	}

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
	if (targetsVal != nullptr)
	{
		if (!(targetsVal->GetType() == core::JSON::ValueType::JSON_Array)) 
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected primitive 'targets' to be a JSON array");
			return false;
		}

		core::JSON::Array* targetsArr = targetsVal->arr();
		for (core::JSON::Value* targetVal : targetsArr->value)
		{
			if (!(targetVal->GetType() == core::JSON::ValueType::JSON_Object)) 
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected primitive 'targets' element to be a JSON object");
				return false;
			}

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
	if (accessorsVal == nullptr)
	{
		return true;
	}

	if (!(accessorsVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'accessors' to be a JSON array");
		return false;
	}

	JSON::Array* accessorsArr = accessorsVal->arr();

	for (JSON::Value* accessorVal : accessorsArr->value)
	{
		if (!(accessorVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'accessors' element to be a JSON object");
			return false;
		}

		JSON::Object* accessorObj = accessorVal->obj();

		Accessor accessor;

		//Required components
		if (!JSON::ApplyIntValue(accessorObj, accessor.componentType, "componentType"))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'componentType' from accessor");
			return false;
		}

		if (!JSON::ApplyIntValue(accessorObj, accessor.count, "count"))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'count' from accessor");
			return false;
		}

		std::string typeStr;
		if (!JSON::ApplyStrValue(accessorObj, typeStr, "type"))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'type' from accessor");
			return false;
		}

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

		if (!typeFound)
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'type' in accessor");
			return false;
		}

		//Rest
		JSON::ApplyIntValue(accessorObj, accessor.bufferView, "bufferView");
		JSON::ApplyIntValue(accessorObj, accessor.byteOffset, "byteOffset");
		JSON::ApplyBoolValue(accessorObj, accessor.normalized, "normalized");
		JSON::ApplyStrValue(accessorObj, accessor.name, "name");

		JSON::Value* maxVal = (*accessorObj)["max"];
		if (maxVal != nullptr)
		{
			if (!(maxVal->GetType() == JSON::ValueType::JSON_Array))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected accessor 'max' to be a JSON array");
				return false;
			}

			JSON::Array* jArr = maxVal->arr();
			std::vector<double> arr = jArr->NumArr();
			for (auto el : arr)
			{
				accessor.max.push_back(static_cast<float>(el));
			}
		}

		JSON::Value* minVal = (*accessorObj)["min"];
		if (minVal != nullptr)
		{
			if (!(minVal->GetType() == JSON::ValueType::JSON_Array))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected accessor 'min' to be a JSON array");
				return false;
			}

			JSON::Array* jArr = minVal->arr();
			std::vector<double> arr = jArr->NumArr();
			for (auto el : arr)
			{
				accessor.min.push_back(static_cast<float>(el));
			}
		}

		accessors.push_back(accessor);

		JSON::Value* sparseVal = (*accessorObj)["sparse"];
		if (sparseVal != nullptr)
		{
			if (!(sparseVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected accessor 'sparse' to be a JSON object");
				return false;
			}

			JSON::Object* sparseObj = sparseVal->obj();

			accessors[accessors.size() - 1].sparse = new Accessor::Sparse();
			Accessor::Sparse* sparse = accessors[accessors.size() - 1].sparse;

			if (!JSON::ApplyIntValue(sparseObj, sparse->count, "count"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'count' from sparse accessor");
				return false;
			}

			// indices
			JSON::Value* indicesVal = (*sparseObj)["indices"];
			if (!indicesVal)
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'indices' from sparse accessor");
				return false;
			}

			if (!(indicesVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF sparse accessor expected 'incices' to be a JSON object");
				return false;
			}

			JSON::Object* indicesObj = indicesVal->obj();
			if (!JSON::ApplyIntValue(indicesObj, sparse->indices.bufferView, "bufferView"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'bufferView' from sparse accessor indices");
				return false;
			}

			JSON::ApplyIntValue(indicesObj, sparse->indices.byteOffset, "byteOffset");
			if (!JSON::ApplyIntValue(indicesObj, sparse->indices.componentType, "componentType"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'componentType' from sparse accessor indices");
				return false;
			}

			// values
			JSON::Value* valuesVal = (*sparseObj)["values"];
			if (valuesVal == nullptr)
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'values' from sparse accessor");
				return false;
			}

			if (!(valuesVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF sparse accessor expected 'values' to be a JSON object");
				return false;
			}

			JSON::Object* valuesObj = valuesVal->obj();
			if (!JSON::ApplyIntValue(valuesObj, sparse->values.bufferView, "bufferView"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'bufferView' from sparse accessor values");
				return false;
			}

			JSON::ApplyIntValue(valuesObj, sparse->values.byteOffset, "byteOffset");
		}
	}

	return true;
}

bool glTF::ParseBufferViewsJson(core::JSON::Object* root, std::vector<BufferView>& bufferViews)
{
	using namespace core;

	JSON::Value* bufferViewsVal = (*root)["bufferViews"];
	if (bufferViewsVal == nullptr)
	{
		return true;
	}

	if (!(bufferViewsVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'bufferViews' to be a JSON array");
		return false;
	}

	JSON::Array* bufferViewsArr = bufferViewsVal->arr();
	for (JSON::Value* bufferViewVal : bufferViewsArr->value)
	{
		if (!(bufferViewVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'bufferViews' element to be a JSON object");
			return false;
		}

		JSON::Object* bufferViewObj = bufferViewVal->obj();

		BufferView view;

		if (!JSON::ApplyIntValue(bufferViewObj, view.buffer, "buffer"))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'buffer' from buffer view");
			return false;
		}

		if (!JSON::ApplyIntValue(bufferViewObj, view.byteLength, "byteLength"))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'byteLength' from buffer view");
			return false;
		}

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
	if (buffersVal == nullptr)
	{
		return true;
	}

	if (!(buffersVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'buffers' to be a JSON array");
		return false;
	}

	JSON::Array* buffersArr = buffersVal->arr();

	for (JSON::Value* bufferVal : buffersArr->value)
	{
		if (!(bufferVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'buffers' element to be a JSON object");
			return false;
		}

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
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'byteLength' from buffer");
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
	if (texturesVal == nullptr)
	{
		return true;
	}

	if (!(texturesVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'textures' to be a JSON array");
		return false;
	}

	JSON::Array* texturesArr = texturesVal->arr();
	for (JSON::Value* textureVal : texturesArr->value)
	{
		if (!(textureVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'textures' element to be a JSON object");
			return false;
		}

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
	if (imagesVal == nullptr)
	{
		return true;
	}

	if (!(imagesVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'images' to be a JSON array");
		return false;
	}

	JSON::Array* imagesArr = imagesVal->arr();
	for (JSON::Value* imageVal : imagesArr->value)
	{
		if (!(imageVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'images' element to be a JSON object");
			return false;
		}

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
	if (samplersVal == nullptr)
	{
		return true;
	}

	if (!(samplersVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'samplers' to be a JSON array");
		return false;
	}

	JSON::Array* samplersArr = samplersVal->arr();
	for (JSON::Value* samplerVal : samplersArr->value)
	{
		if (!(samplerVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'samplers' element to be a JSON object");
			return false;
		}

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
	if (materialsVal == nullptr)
	{
		return true;
	}

	if (!(materialsVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'materials' element to be a JSON array");
		return false;
	}

	JSON::Array* materialsArr = materialsVal->arr();

	for (JSON::Value* materialVal : materialsArr->value)
	{
		if (!(materialVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'materials' element to be a JSON object");
			return false;
		}

		JSON::Object* materialObj = materialVal->obj();

		materials.push_back(Material());
		Material* material = &materials[materials.size() - 1];

		JSON::ApplyStrValue(materialObj, material->name, "name");

		ET_ASSERT(material->pbrMetallicRoughness == nullptr);
		material->pbrMetallicRoughness = new Material::PbrMetallicRoughness();
		JSON::Value* pbrMetallicRoughnessVal = (*materialObj)["pbrMetallicRoughness"];
		if (pbrMetallicRoughnessVal != nullptr)
		{
			if (!(pbrMetallicRoughnessVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected materials 'pbrMetallicRoughness' to be a JSON object");
				return false;
			}

			if (!ParsePbrMetallicRoughnessJson(pbrMetallicRoughnessVal->obj(), material->pbrMetallicRoughness))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to parse pbrMetallicRoughness object");
				return false;
			}
		}

		JSON::Value* normalTextureVal = (*materialObj)["normalTexture"];
		if (normalTextureVal != nullptr)
		{
			if (!(normalTextureVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected materials 'normalTexture' to be a JSON object");
				return false;
			}

			JSON::Object* normalTextureObj = normalTextureVal->obj();

			ET_ASSERT(material->normalTexture == nullptr);
			material->normalTexture = new Material::NormalTextureInfo();
			if (!ParseTextureInfoJson(normalTextureObj, material->normalTexture))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to parse materials texture info for 'normalTexture'");
				return false;
			}

			if (material->normalTexture)
			{
				JSON::ApplyNumValue(normalTextureObj, material->normalTexture->scale, "scale");
			}
		}

		JSON::Value* occlusionTextureVal = (*materialObj)["occlusionTexture"];
		if (occlusionTextureVal != nullptr)
		{
			if (!(occlusionTextureVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected materials 'occlusionTexture' to be a JSON object");
				return false;
			}

			JSON::Object* occlusionTextureObj = occlusionTextureVal->obj();
			
			ET_ASSERT(material->occlusionTexture == nullptr);
			material->occlusionTexture = new Material::OcclusionTextureInfo();
			if (!ParseTextureInfoJson(occlusionTextureObj, material->occlusionTexture))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to parse materials texture info for 'occlusionTexture'");
				return false;
			}

			if (material->occlusionTexture)
			{
				JSON::ApplyNumValue(occlusionTextureObj, material->occlusionTexture->strength, "strength");
			}
		}

		JSON::Value* emissiveTextureVal = (*materialObj)["emissiveTexture"];
		if (emissiveTextureVal != nullptr)
		{
			if (!(emissiveTextureVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected materials 'emissiveTexture' to be a JSON object");
				return false;
			}

			ET_ASSERT(material->emissiveTexture == nullptr);
			material->emissiveTexture = new Material::TextureInfo();
			if (!ParseTextureInfoJson(emissiveTextureVal->obj(), material->emissiveTexture))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to parse materials texture info for 'emissiveTexture'");
				return false;
			}
		}

		JSON::Value* emissiveFactorVal = (*materialObj)["emissiveFactor"];
		if (emissiveFactorVal != nullptr)
		{
			if (!JSON::ArrayVector(emissiveFactorVal, material->emissiveFactor))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'emissiveFactor' from material");
				return false;
			}
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

			if (!alphaModeFound)
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'alphaMode' on material");
				return false;
			}
		}

		JSON::ApplyNumValue(materialObj, material->alphaCutoff, "alphaCutoff");
		JSON::ApplyBoolValue(materialObj, material->doubleSided, "doubleSided");
	}

	return true;
}

bool glTF::ParsePbrMetallicRoughnessJson(core::JSON::Object* pbrObj, Material::PbrMetallicRoughness* pbr)
{
	using namespace core;

	ET_ASSERT(pbrObj != nullptr);

	JSON::Value* baseColorFactorVal = (*pbrObj)["baseColorFactor"];
	if (baseColorFactorVal != nullptr)
	{
		if (!JSON::ArrayVector(baseColorFactorVal, pbr->baseColorFactor))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'baseColorFactor' from PBR material");
			return false;
		}
	}

	JSON::Value* baseColorTextureVal = (*pbrObj)["baseColorTexture"];
	if (baseColorTextureVal != nullptr)
	{
		if (!(baseColorTextureVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'baseColorTexture' on PBR material to be a JSON object");
			return false;
		}

		ET_ASSERT(pbr->baseColorTexture == nullptr);
		pbr->baseColorTexture = new Material::TextureInfo();
		if (!ParseTextureInfoJson(baseColorTextureVal->obj(), pbr->baseColorTexture))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to parse PBR materials texture info for 'baseColorTexture'");
			return false;
		}
	}

	JSON::ApplyNumValue(pbrObj, pbr->metallicFactor, "metallicFactor");

	JSON::ApplyNumValue(pbrObj, pbr->roughnessFactor, "roughnessFactor");

	JSON::Value* metallicRoughnessTextureVal = (*pbrObj)["metallicRoughnessTexture"];
	if (metallicRoughnessTextureVal != nullptr)
	{
		if (!(metallicRoughnessTextureVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'metallicRoughnessTexture' on PBR material to be a JSON object");
			return false;
		}

		ET_ASSERT(pbr->metallicRoughnessTexture == nullptr);
		pbr->metallicRoughnessTexture = new Material::TextureInfo();
		if (!ParseTextureInfoJson(metallicRoughnessTextureVal->obj(), pbr->metallicRoughnessTexture))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to parse PBR materials texture info for 'metallicRoughnessTexture'");
			return false;
		}
	}

	return true;
}

bool glTF::ParseTextureInfoJson(core::JSON::Object* textureInfo, Material::TextureInfo* info)
{
	using namespace core;

	ET_ASSERT(info != nullptr);

	if (!JSON::ApplyIntValue(textureInfo, info->index, "index"))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'index' from texture info");
		return false;
	}

	JSON::ApplyIntValue(textureInfo, info->texCoord, "texCoord");

	return true;
}

bool glTF::ParseCamerasJson(core::JSON::Object* root, std::vector<Camera>& cameras)
{
	using namespace core;

	JSON::Value* camerasVal = (*root)["cameras"];
	if (camerasVal == nullptr)
	{
		return true;
	}

	if (!(camerasVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'cameras' element to be a JSON array");
		return false;
	}

	JSON::Array* camerasArr = camerasVal->arr();
	for (JSON::Value* cameraVal : camerasArr->value)
	{
		if (!(cameraVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'cameras' element to be a JSON object");
			return false;
		}

		JSON::Object* cameraObj = cameraVal->obj();

		cameras.push_back(Camera());
		Camera* camera = &cameras[cameras.size() - 1];

		std::string cameraTypeStr;
		if (!(JSON::ApplyStrValue(cameraObj, cameraTypeStr, "type")))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'type' from camera");
			return false;
		}

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

		if (!cameraTypeFound)
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'type' on camera");
			return false;
		}

		JSON::ApplyStrValue(cameraObj, camera->name, "name");

		JSON::Value* orthographicVal = (*cameraObj)["orthographic"];
		if (orthographicVal)
		{
			if (!(orthographicVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected camera 'orthographic' to be a JSON object");
				return false;
			}

			JSON::Object* orthographicObj = orthographicVal->obj();

			ET_ASSERT(camera->orthographic == nullptr);
			camera->orthographic = new Camera::Orthographic();

			if (!JSON::ApplyNumValue(orthographicObj, camera->orthographic->xmag, "xmag"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'xmag' from orthographic camera");
				return false;
			}

			if (!JSON::ApplyNumValue(orthographicObj, camera->orthographic->yamg, "yamg"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'yamg' from orthographic camera");
				return false;
			}

			if (!JSON::ApplyNumValue(orthographicObj, camera->orthographic->zfar, "zfar"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'zfar' from orthographic camera");
				return false;
			}

			if (!JSON::ApplyNumValue(orthographicObj, camera->orthographic->znear, "znear"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'znear' from orthographic camera");
				return false;
			}
		}

		JSON::Value* perspectiveVal = (*cameraObj)["perspective"];
		if (perspectiveVal)
		{
			if (!(perspectiveVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected camera 'perspective' to be a JSON object");
				return false;
			}

			JSON::Object* perspectiveObj = perspectiveVal->obj();

			ET_ASSERT(camera->perspective == nullptr);
			camera->perspective = new Camera::Perspective();

			JSON::ApplyNumValue(perspectiveObj, camera->perspective->aspectRatio, "aspectRatio");
			if (!JSON::ApplyNumValue(perspectiveObj, camera->perspective->yfov, "yfov"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'yfov' from perspective camera");
				return false;
			}

			JSON::ApplyNumValue(perspectiveObj, camera->perspective->zfar, "zfar");
			if (!JSON::ApplyNumValue(perspectiveObj, camera->perspective->znear, "znear"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'znear' from perspective camera");
				return false;
			}
		}
	}

	return true;
}

bool glTF::ParseSkinsJson(core::JSON::Object* root, std::vector<Skin>& skins)
{
	using namespace core;

	JSON::Value* skinsVal = (*root)["skins"];
	if (skinsVal == nullptr)
	{
		return true;
	}

	if (!(skinsVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'skins' element to be a JSON array");
		return false;
	}

	JSON::Array* skinsArr = skinsVal->arr();
	for (JSON::Value* skinVal : skinsArr->value)
	{
		if (!(skinVal->GetType() == JSON::ValueType::JSON_Object))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'skins' element to be a JSON object");
			return false;
		}

		JSON::Object* skinObj = skinVal->obj();

		Skin skin;

		JSON::Value* jointsVal = (*skinObj)["joints"];
		if (jointsVal == nullptr)
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'joints' on skin");
			return false;
		}

		if (!(jointsVal->GetType() == JSON::ValueType::JSON_Array))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected skin 'joints' to be a JSON array");
			return false;
		}

		JSON::Array* jointsArr = jointsVal->arr();
		std::vector<int64> intArr = jointsArr->IntArr();
		for (auto el : intArr)
		{
			skin.joints.push_back(static_cast<uint32>(el));
		}

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
	if (animationsVal == nullptr)
	{
		return true;
	}

	if (!(animationsVal->GetType() == JSON::ValueType::JSON_Array))
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'animations' element to be a JSON array");
		return false;
	}

	JSON::Array* animationsArr = animationsVal->arr();
	for (JSON::Value* animationVal : animationsArr->value)
	{
		if (!(animationVal->GetType() == JSON::ValueType::JSON_Object)) 
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected 'animations' element to be a JSON object");
			return false;
		}

		JSON::Object* animationObj = animationVal->obj();

		Animation animation;

		JSON::ApplyStrValue(animationObj, animation.name, "name");

		JSON::Value* channelsVal = (*animationObj)["channels"];
		if (channelsVal == nullptr)
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'channels' on animation");
			return false;
		}

		if (!(channelsVal->GetType() == JSON::ValueType::JSON_Array))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected animation 'channels' to be a JSON array");
			return false;
		}

		JSON::Array* channelsArr = channelsVal->arr();
		for (JSON::Value* channelVal : channelsArr->value)
		{
			if (!(channelVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected animation 'channels' element to be a JSON object");
				return false;
			}

			JSON::Object* channelObj = channelVal->obj();

			Animation::Channel channel;

			if (!JSON::ApplyIntValue(channelObj, channel.sampler, "sampler"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'sampler' from animation channel");
				return false;
			}

			JSON::Value* targetVal = (*channelObj)["target"];
			if (!targetVal)
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'target' on animation channel");
				return false;
			}

			if (!(targetVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected animation channel 'target' to be a JSON object");
				return false;
			}

			JSON::Object* targetObj = targetVal->obj();

			JSON::ApplyIntValue(targetObj, channel.target.node, "node");

			std::string targetPathStr;
			if (!(JSON::ApplyStrValue(targetObj, targetPathStr, "path")))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'path' from animation channel target");
				return false;
			}

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

			if (!targetPathFound)
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'path' on animation channel target");
				return false;
			}

			animation.channels.push_back(channel);
		}

		JSON::Value* samplersVal = (*animationObj)["samplers"];
		if (samplersVal == nullptr)
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'samplers' on animation");
			return false;
		}

		if (!(samplersVal->GetType() == JSON::ValueType::JSON_Array))
		{
			ET_LOG_E(ET_CTX_EDITOR, "glTF expected animation 'samplers' to be a JSON array");
			return false;
		}

		JSON::Array* samplersArr = samplersVal->arr();
		for (JSON::Value* samplerVal : samplersArr->value)
		{
			if (!(samplerVal->GetType() == JSON::ValueType::JSON_Object))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF expected animation 'samplers' element to be a JSON object");
				return false;
			}

			JSON::Object* samplerObj = samplerVal->obj();

			Animation::Sampler sampler;

			if (!JSON::ApplyIntValue(samplerObj, sampler.input, "input"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'input' from animation sampler");
				return false;
			}

			if (!JSON::ApplyIntValue(samplerObj, sampler.output, "output"))
			{
				ET_LOG_E(ET_CTX_EDITOR, "glTF failed to read 'output' from animation sampler");
				return false;
			}

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

				if (!samplerInterpolationFound)
				{
					ET_LOG_E(ET_CTX_EDITOR, "glTF failed to find 'interpolation' on animation channel sampler");
					return false;
				}
			}

			animation.samplers.push_back(sampler);
		}

		animations.push_back(animation);
	}

	return true;
}


} // namespace edit
} // namespace et
