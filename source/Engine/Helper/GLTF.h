#pragma once

namespace GLTF
{
	struct Primitive
	{
		uint32 mode = 0;
		uint32 indices = 0;
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
	};
	typedef std::vector<Primitive> Mesh;

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
		BYTE			= 5120,
		UNSIGNED_BYTE	= 5121,
		SHORT			= 5122,
		UNSIGNED_SHORT	= 5123,
		UNSIGNED_INT	= 5125,
		FLOAT			= 5126
	};

	enum class Type
	{
		SCALAR,
		VEC2,
		VEC3,
		VEC4,
		MAT2,
		MAT3,
		MAT4
	};

	struct Accessor
	{
		uint32 bufferView = 0;
		uint32 byteOffset = 0;
		ComponentType componentType = ComponentType::BYTE;
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
		}*sparse = nullptr;

		std::vector<float> max;
		std::vector<float> min;

		~Accessor() { delete sparse; }
	};

	struct Dom
	{
		std::vector<Mesh> meshes;
		std::vector<Buffer> buffers;
		std::vector<BufferView> bufferViews;
		std::vector<Accessor> accessors;
	};
}
