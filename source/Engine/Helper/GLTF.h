#pragma once
#include "../Math/Quaternion.hpp"
#include "../FileSystem/JSONdom.h"

class MeshFilter;
class BinaryReader;

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

	struct URI
	{
		std::string path;
		std::vector<uint8> binData;
		std::string ext;
		enum Type : uint8
		{
			URI_UNEVALUATED,
			URI_FILE,
			URI_DATA,
			URI_NONE
		} type = URI_UNEVALUATED;
	};

	struct Buffer
	{
		uint64 byteLength = 0;
		URI uri;
		std::string name;
	};

	struct BufferView
	{
		uint32 buffer = 0;
		uint64 byteLength = 0;
		uint64 byteOffset = 0;
		int32 byteStride = -1;
		int32 target = -1;
		std::string name;
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
	static std::map<ComponentType, uint8> ComponentTypes
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
	static std::map<Type, std::pair<uint8, std::string>> AccessorTypes
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
		int32 inverseBindMatrices = -1;
		std::vector<uint32> joints;
		int32 skeleton = -1;
	};

	struct Texture
	{
		uint32 sampler = 0;
		uint32 source = 0;
		std::string name;
	};

	struct Image
	{
		URI uri;
		int32 bufferView = -1;
		std::string mimeType;
		std::string name;
	};

	enum class Filter : int32
	{
		UNDEFINED				= -1,
		NEAREST					= 9728,
		LINEAR					= 9729,
		NEAREST_MIPMAP_NEAREST	= 9984,
		LINEAR_MIPMAP_NEAREST	= 9985,
		NEAREST_MIPMAP_LINEAR	= 9986,
		LINEAR_MIPMAP_LINEAR	= 9987
	};
	enum class WrappingMode : uint32
	{
		CLAMP_TO_EDGE	= 33071,
		MIRRORED_REPEAT = 33648,
		REPEAT			= 10497
	};
	struct Sampler
	{
		Filter magFilter = Filter::UNDEFINED;
		Filter minFilter = Filter::UNDEFINED;
		WrappingMode wrapS = WrappingMode::REPEAT;
		WrappingMode wrapT = WrappingMode::REPEAT;
		std::string name;
	};

	enum class AlphaMode : uint8 { GLTF_OPAQUE, GLTF_MASK, GLTF_BLEND };
	static const std::map<AlphaMode, std::string> AlphaModes
	{
		{ AlphaMode::GLTF_OPAQUE, "OPAQUE" },
		{ AlphaMode::GLTF_MASK, "MASK" },
		{ AlphaMode::GLTF_BLEND, "BLEND" }
	};
	struct Material
	{
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
			float metallicFactor = 1;
			float roughnessFactor = 1;
			TextureInfo* baseColorTexture = nullptr;//sRGB
			TextureInfo* metallicRoughnessTexture = nullptr;//linear

			~PbrMetallicRoughness() { delete baseColorTexture; delete metallicRoughnessTexture; }
		};
		PbrMetallicRoughness *pbrMetallicRoughness = nullptr;

		NormalTextureInfo *normalTexture = nullptr;//linear
		OcclusionTextureInfo *occlusionTexture = nullptr;//linear
		TextureInfo* emissiveTexture = nullptr;//sRGB
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
		struct Perspective
		{
			float aspectRatio = -1;
			float yfov = 0;
			float zfar = -1;
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
	static const std::map<Camera::Type, std::string> CameraTypes
	{
		{ Camera::Type::PERSPECTIVE, "perspective" },
		{ Camera::Type::ORTHOGRAPHIC, "orthographic" }
	};

	struct Animation
	{
		std::string name;
		struct Channel
		{
			uint32 sampler = 0;
			struct Target
			{
				int32 node = -1;
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
	static const std::map<Animation::Channel::Target::Path, std::string> ChannelTargetPaths
	{
		{ Animation::Channel::Target::Path::TRANSLATION, "translation" },
		{ Animation::Channel::Target::Path::ROTATION, "rotation" },
		{ Animation::Channel::Target::Path::SCALE, "scale" },
		{ Animation::Channel::Target::Path::WEIGHTS, "weights" }
	};
	static const std::map<Animation::Sampler::Interpolation, std::string> SamplerInterpolations
	{
		{ Animation::Sampler::Interpolation::LINEAR, "LINEAR" },
		{ Animation::Sampler::Interpolation::STEP, "STEP" },
		{ Animation::Sampler::Interpolation::CUBISPLINE, "CUBISPLINE" }
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
		std::vector<Animation> animations;
		std::vector<Texture> textures;
		std::vector<Image> images;
		std::vector<Sampler> samplers;
		std::vector<Material> materials;
		std::vector<Camera> cameras;
	};

	//For binary glTF / GLB

	struct Header
	{
		uint32 magic = *reinterpret_cast<uint32*>("glTF");	// should be "glTF"
		uint32 version = 2; // should be 2
		uint32 length = 0;
	};

	struct Chunk
	{
		uint32 chunkLength = 0;
		enum class ChunkType : uint32 
		{
			JSON	= 0x4E4F534A,
			BIN		= 0x004E4942
		} chunkType;
		std::vector<uint8> chunkData;
	};

	struct glTFAsset
	{
		Header header;
		Dom dom;
		std::vector<Chunk> dataChunks;
		std::string basePath;
	};

	static const std::string Base64Mime = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static inline bool IsBase64(unsigned char c) 
	{
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	bool EvaluateURI(URI& uri, const std::string& basePath);
	bool DecodeBase64(const std::string& encoded, std::vector<uint8>& decoded);

	//Unify GLTF and GLB
	bool ParseGLTFData(const std::vector<uint8>& binaryContent, const std::string path, const std::string& extension, glTFAsset& asset);

	bool ParseGLBHeader(BinaryReader* pBinReader, Header &header);
	bool ParseGLBChunk(BinaryReader* pBinReader, Chunk &header);

	bool ParseGlTFJson(JSON::Object* json, Dom& dom);

	bool ParseAssetJson(JSON::Object* root, Asset& asset);
	bool ParseExtensionsJson(JSON::Object* root, Dom& dom);
	bool ParseSceneJson(JSON::Object* root, Dom& dom);
	bool ParseScenesJson(JSON::Object* root, std::vector<Scene>& scenes);
	bool ParseNodesJson(JSON::Object* root, std::vector<Node>& nodes);
	bool ParseMeshesJson(JSON::Object* root, std::vector<Mesh>& meshes);
	bool ParsePrimitiveJson(JSON::Object* primitiveObj, Primitive& primitive);
	bool ParseAccessorsJson(JSON::Object* root, std::vector<Accessor>& accessors);
	bool ParseBufferViewsJson(JSON::Object* root, std::vector<BufferView>& bufferViews);
	bool ParseBuffersJson(JSON::Object* root, std::vector<Buffer>& buffers);
	bool ParseTexturesJson(JSON::Object* root, std::vector<Texture>& textures);
	bool ParseImagesJson(JSON::Object* root, std::vector<Image>& images);
	bool ParseSamplersJson(JSON::Object* root, std::vector<Sampler>& samplers);
	bool ParseMaterialsJson(JSON::Object* root, std::vector<Material>& materials);
	bool ParsePbrMetallicRoughnessJson(JSON::Object* pbrObj, Material::PbrMetallicRoughness* pbr);
	bool ParseTextureInfoJson(JSON::Object* textureInfo, Material::TextureInfo* info);
	bool ParseCamerasJson(JSON::Object* root, std::vector<Camera>& cameras);
	bool ParseSkinsJson(JSON::Object* root, std::vector<Skin>& skins);
	bool ParseAnimationsJson(JSON::Object* root, std::vector<Animation>& animations);

	void LogGLTFVersionSupport();

	bool OpenBufferViewReader(glTFAsset& asset, uint32 viewIdx, BinaryReader* pViewReader);
	bool GetAccessorData(glTFAsset& asset, uint32 idx, std::vector<uint8>& data);
	template<typename T>
	bool GetAccessorScalarArray(glTFAsset& asset, uint32 idx, std::vector<T>& data)
	{
		if (idx >= (uint32)asset.dom.accessors.size())
		{
			LOG("Accessor index out of range", Warning);
			return false;
		}
		Accessor& accessor = asset.dom.accessors[idx];
		uint8 compsPerEl = AccessorTypes[accessor.type].first;
		std::vector<uint8> accessorData;
		if (!GetAccessorData(asset, idx, accessorData))
		{
			LOG("Unable to get accessor data", Warning);
		}
		BinaryReader* pBinReader = new BinaryReader();
		pBinReader->Open(accessorData);
		if (!(pBinReader->Exists()))
		{
			LOG("Unable to convert accessor data", Warning);
			delete pBinReader;
			return false;
		}
		for (uint32 i = 0; i < accessor.count * compsPerEl; ++i)
		{
			if (pBinReader->GetBufferPosition() >= (int32)accessorData.size())
			{
				LOG("Binary reader out of range", Warning);
				delete pBinReader;
				return false;
			}
			switch (accessor.componentType)
			{
			case ComponentType::BYTE:
				data.push_back(static_cast<T>(pBinReader->Read<int8>()));
				break;
			case ComponentType::UNSIGNED_BYTE:
				data.push_back(static_cast<T>(pBinReader->Read<uint8>()));
				break;
			case ComponentType::SHORT:
				data.push_back(static_cast<T>(pBinReader->Read<int16>()));
				break;
			case ComponentType::UNSIGNED_SHORT:
				data.push_back(static_cast<T>(pBinReader->Read<uint16>()));
				break;
			case ComponentType::UNSIGNED_INT:
				data.push_back(static_cast<T>(pBinReader->Read<uint32>()));
				break;
			case ComponentType::FLOAT:
				data.push_back(static_cast<T>(pBinReader->Read<float>()));
				break;
			}
		}
		delete pBinReader;
		return true;
	}
	template <uint8 n, class T>
	bool GetAccessorVectorArray(glTFAsset& asset, uint32 idx, std::vector<etm::vector<n, T>>& data, bool convertCoords = false)
	{
		if (idx >= (uint32)asset.dom.accessors.size())
		{
			LOG("Accessor index out of range", Warning);
			return false;
		}
		Accessor& accessor = asset.dom.accessors[idx];
		uint8 compsPerEl = AccessorTypes[accessor.type].first;
		if (compsPerEl != n)
		{
			LOG("Accessor type mismatch with vector size", Warning);
			return false;
		}
		std::vector<T> scalars;
		if(!GetAccessorScalarArray(asset, idx, scalars))
		{
			LOG("Unable to get accessor scalar array for vector array", Warning);
			return false;
		}
		if (convertCoords && n != 3)
		{
			LOG("Converting coordinates of a non-3D vector", Warning);
		}
		convertCoords &= n > 1;
		for (uint32 i = 0; i < scalars.size() / n; ++i)
		{
			etm::vector<n, T> vec;
			for (uint32 j = 0; j < n; ++j)
			{
				vec[(uint8)j] = scalars[i*(uint32)n + j];
			}
			if (convertCoords)vec[1] = -vec[1];
			data.push_back(vec);
		}
		return true;
	}
	class MeshFilterConstructor //Allows inner access to mesh filters through friend class
	{
	public:
		static bool GetMeshFilters(glTFAsset& asset, std::vector<MeshFilter*>& meshFilters);
	};
}