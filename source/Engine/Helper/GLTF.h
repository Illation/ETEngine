#pragma once
#include "AtomicTypes.hpp"
#include <vector>
#include "../Math/Vector.hpp"
#include "../Math/Quaternion.hpp"
#include "../Math/Matrix.hpp"
#include "../FileSystem/JSONdom.h"

namespace glTF
{
	static const float minVersion = 2.0;
	static const float maxVersion = 2.0;
	static const std::vector<std::string> supportedExtensions
	{
	};

	struct Asset
	{
		std::string version;
		std::string minVersion;
		std::string generator;
		std::string copyright;
	};

	//coordinates: gltf->etm = x, y, -z
	//the dom will store the coordinates in gltf format

	struct Node
	{
		std::string name;
		std::vector<uint32> children;
		vec3 translation = vec3();
		quat rotation = quat();
		vec3 scale = vec3(1);
		mat4 matrix = mat4();
		int32 camera = -1;
		int32 mesh = -1;
		int32 skin = -1;
		std::vector<float> weights;
	};

	struct Scene
	{
		std::string name;
		std::vector<uint32> nodes;
	};

	struct Primitive
	{
		uint32 mode = 4;
		int32 indices = -1;
		struct Attributes
		{
			int32 position = -1;
			int32 normal = -1;
			int32 tangent = -1;
			int32 texcoord0 = -1;
			int32 texcoord1 = -1;
			int32 color0 = -1;
			int32 joints0 = -1;
			int32 weights0 = -1;
		} attributes;
		int32 material = -1;
		struct Targets
		{
			int32 position = -1;
			int32 normal = -1;
			int32 tangent = -1;
		};
		std::vector<Targets> targets;
	};

	struct Mesh
	{
		std::string name;
		std::vector<Primitive> primitives;
		std::vector<float> weights;
	};

	struct Buffer
	{
		uint64 byteLength = 0;
		std::string uri;
	};

	struct BufferView
	{
		uint32 buffer = 0;
		uint64 byteLength = 0;
		uint64 byteOffset = 0;
		int32 byteStride = -1;
		int32 target = -1;
	};

	enum class ComponentType : uint32
	{
		BYTE = 5120,
		UNSIGNED_BYTE = 5121,
		SHORT = 5122,
		UNSIGNED_SHORT = 5123,
		UNSIGNED_INT = 5125,
		FLOAT = 5126
	};
	static const std::map<ComponentType, uint8> ComponentTypes
	{
		{ ComponentType::BYTE,			 1 },
		{ ComponentType::UNSIGNED_BYTE,	 1 },
		{ ComponentType::SHORT,			 2 },
		{ ComponentType::UNSIGNED_SHORT, 2 },
		{ ComponentType::UNSIGNED_INT,	 4 },
		{ ComponentType::FLOAT,			 4 }
	};

	enum class Type
	{
		SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4
	};
	static const std::map<Type, std::pair<uint8, std::string>> AccessorTypes
	{
		{ Type::SCALAR,	{ 1, "SCALAR" } },
		{ Type::VEC2,	{ 2, "VEC2" } },
		{ Type::VEC3,	{ 3, "VEC3" } },
		{ Type::VEC4,	{ 4, "VEC4" } },
		{ Type::MAT2,	{ 4, "MAT2" } },
		{ Type::MAT3,	{ 9, "MAT3" } },
		{ Type::MAT4,	{ 16, "MAT4" } }
	};
	struct Accessor
	{
		int32 bufferView = -1;
		uint32 byteOffset = 0;
		ComponentType componentType = ComponentType::BYTE;
		bool normalized = false;
		uint64 count = 0;
		Type type = Type::VEC3;

		struct Sparse
		{
			uint64 count = 0;
			struct Indices
			{
				uint32 bufferView = 0;
				uint32 byteOffset = 0;
				ComponentType componentType = ComponentType::BYTE;
			}indices;
			struct Values
			{
				uint32 bufferView = 0;
				uint32 byteOffset = 0;
			}values;
		} *sparse = nullptr;

		std::vector<float> max;
		std::vector<float> min;

		std::string name;

		~Accessor() { delete sparse; }
	};

	struct Skin
	{
		std::string name;
		uint32 inverseBindMatrices = 0;
		std::vector<uint32> joints;
		uint32 skeleton = 0;
	};

	struct Texture
	{
		uint32 sampler = 0;
		uint32 source = 0;
	};

	struct Image
	{
		std::string uri;
		int32 bufferView = -1;
		std::string mimeType;
	};

	struct Sampler
	{
		uint32 magFilter = 0;
		uint32 minFilter = 0;
		uint32 wrapS = 0;
		uint32 wrapT = 0;
	};
	
	struct Material
	{
		enum class AlphaMode : uint8
		{
			GLTF_OPAQUE,
			GLTF_MASK,
			GLTF_BLEND
		};
		struct TextureInfo
		{
			uint32 index = 0;
			uint32 texCoord = 0;
		};
		struct NormalTextureInfo : TextureInfo
		{
			float scale = 1;
		};
		struct OcclusionTextureInfo : TextureInfo
		{
			float strength = 1;
		};

		std::string name;
		struct PbrMetallicRoughness
		{
			vec4 baseColorFactor = vec4(1);
			float metallicFactor = 0;
			float roughnessFactor = 0;
			TextureInfo* baseColorTexture = nullptr;
			TextureInfo* metallicRoughnessTexture = nullptr;

			~PbrMetallicRoughness() { delete baseColorTexture; delete metallicRoughnessTexture; }
		};
		PbrMetallicRoughness *pbrMetallicRoughness = nullptr;

		NormalTextureInfo *normalTexture = nullptr;
		OcclusionTextureInfo *occlusionTexture = nullptr;
		TextureInfo* emissiveTexture = nullptr;
		vec3 emissiveFactor = vec3(0);
		AlphaMode alphaMode = AlphaMode::GLTF_OPAQUE;
		float alphaCutoff = 0.5f;
		bool doubleSided = false;

		~Material() { delete pbrMetallicRoughness; delete normalTexture; delete occlusionTexture; delete emissiveTexture; }
	};

	struct Camera
	{
		std::string name;
		enum class Type : uint8
		{
			PERSPECTIVE,
			ORTHOGRAPHIC
		} type;
		struct Perpective
		{
			float aspectRatio = 0;
			float yfov = 0;
			float zfar = 0;
			float znear = 0;
		}* perspective = nullptr;
		struct Orthographic
		{
			float xmag = 0;
			float yamg = 0;
			float zfar = 0;
			float znear = 0;
		}* orthographic = nullptr;
	};

	struct Animation
	{
		std::string name;
		struct Channel
		{
			uint32 sampler = 0;
			struct Target
			{
				uint32 node = 0;
				enum class Path : uint8 
				{
					TRANSLATION,
					ROTATION,
					SCALE,
					WEIGHTS
				} path;
			}target;
		};
		std::vector<Channel> channels;
		struct Sampler
		{
			uint32 input = 0;
			enum class Interpolation : uint8 
			{
				LINEAR,
				STEP,
				CUBISPLINE
			} interpolation = Interpolation::LINEAR;
			uint32 output = 0;
		};
		std::vector<Sampler> samplers;
	};

	struct Dom
	{
		Asset asset;

		int32 scene = -1;

		std::vector<std::string> extensionsUsed;
		std::vector<std::string> extensionsRequired;

		std::vector<Node> nodes;
		std::vector<Scene> scenes;
		std::vector<Mesh> meshes;
		std::vector<Buffer> buffers;
		std::vector<BufferView> bufferViews;
		std::vector<Accessor> accessors;
		std::vector<Skin> skins;
		std::vector<Texture> textures;
		std::vector<Image> images;
		std::vector<Sampler> samplers;
		std::vector<Material> materials;
		std::vector<Camera> cameras;
	};

	//For binary glTF / GLB

	struct Header
	{
		uint32 magic;	// should be "glTF"
		uint32 version; // should be 2
		uint32 length;
	};

	struct Chunk
	{
		uint32 chunkLength = 0;
		enum class ChunkType : uint8 
		{
			JSON,
			BIN
		} chunkType;
		std::vector<uint8> chunkData;
	};

	static const std::string Base64Mime = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static inline bool IsBase64(unsigned char c) 
	{
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	bool EvaluateURI(const std::string& uri, std::vector<uint8>& binData, std::string& ext, const std::string& basePath);
	bool DecodeBase64(const std::string& encoded, std::vector<uint8>& decoded);

	bool ParseGlTFJson(JSON::Object* json, Dom& dom);

	bool ParseAssetJson(JSON::Object* root, Asset& asset);
	bool ParseExtensionsJson(JSON::Object* root, Dom& dom);
	bool ParseSceneJson(JSON::Object* root, Dom& dom);
	bool ParseScenesJson(JSON::Object* root, std::vector<Scene>& scenes);
	bool ParseNodesJson(JSON::Object* root, std::vector<Node>& nodes);
	bool ParseMeshesJson(JSON::Object* root, std::vector<Mesh>& meshes);
	bool ParsePrimitiveJson(JSON::Object* primitiveObj, Primitive& primitive);
	bool ParseAccessorsJson(JSON::Object* root, std::vector<Accessor>& accessors);

	void LogGLTFVersionSupport();
}