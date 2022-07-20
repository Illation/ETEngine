#pragma once
#include <EtCore/IO/JsonDom.h>

#include "GltfDom.h"


namespace et {
namespace edit {

namespace glTF {


//Unify GLTF and GLB
bool ParseGLTFData(const std::vector<uint8>& binaryContent, const std::string path, const std::string& extension, glTFAsset& asset);

bool ParseGLBHeader(core::BinaryReader* pBinReader, Header &header);
bool ParseGLBChunk(core::BinaryReader* pBinReader, Chunk &header);

bool ParseGlTFJson(core::JSON::Object* json, Dom& dom);

bool ParseAssetJson(core::JSON::Object* root, Asset& asset);
bool ParseExtensionsJson(core::JSON::Object* root, Dom& dom);
bool ParseSceneJson(core::JSON::Object* root, Dom& dom);
bool ParseScenesJson(core::JSON::Object* root, std::vector<Scene>& scenes);
bool ParseNodesJson(core::JSON::Object* root, std::vector<Node>& nodes);
bool ParseMeshesJson(core::JSON::Object* root, std::vector<Mesh>& meshes);
bool ParsePrimitiveJson(core::JSON::Object* primitiveObj, Primitive& primitive);
bool ParseAccessorsJson(core::JSON::Object* root, std::vector<Accessor>& accessors);
bool ParseBufferViewsJson(core::JSON::Object* root, std::vector<BufferView>& bufferViews);
bool ParseBuffersJson(core::JSON::Object* root, std::vector<Buffer>& buffers);
bool ParseTexturesJson(core::JSON::Object* root, std::vector<Texture>& textures);
bool ParseImagesJson(core::JSON::Object* root, std::vector<Image>& images);
bool ParseSamplersJson(core::JSON::Object* root, std::vector<Sampler>& samplers);
bool ParseMaterialsJson(core::JSON::Object* root, std::vector<Material>& materials);
bool ParsePbrMetallicRoughnessJson(core::JSON::Object* pbrObj, Material::PbrMetallicRoughness* pbr);
bool ParseTextureInfoJson(core::JSON::Object* textureInfo, Material::TextureInfo* info);
bool ParseCamerasJson(core::JSON::Object* root, std::vector<Camera>& cameras);
bool ParseSkinsJson(core::JSON::Object* root, std::vector<Skin>& skins);
bool ParseAnimationsJson(core::JSON::Object* root, std::vector<Animation>& animations);


} // namespace glTF

} // namespace edit
} // namespace et
