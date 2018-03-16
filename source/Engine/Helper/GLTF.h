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

	struct Dom
	{
		std::vector<Mesh> meshes;
	};
}
