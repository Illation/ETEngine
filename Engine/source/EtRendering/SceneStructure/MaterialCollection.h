#pragma once
#include "NodeIdFwd.h"

#include <EtMath/Geometry.h>

#include <EtCore/Content/AssetPointer.h>

#include <EtRHI/GraphicsContext/GraphicsTypes.h>


namespace et {
namespace render {


class rhi::ShaderData;
class I_Material;


//----------------------
// MaterialCollection
//
// List of all meshes to be drawn with a particular shader
//
class MaterialCollection
{
public:

	//--------------------------
	// MaterialCollection::Mesh
	//
	// Mesh draw data and a list of all transformations of its instances
	//
	struct Mesh
	{
		rhi::T_ArrayLoc m_VAO;
		uint32 m_IndexCount;
		rhi::E_DataType m_IndexDataType;
		math::Sphere m_BoundingVolume;
		std::vector<T_NodeId> m_Instances;
	};

	//---------------------------------------
	// MaterialCollection::MaterialInstance
	//
	// Parameters for a shader and list of all meshes to be drawn with these parameters
	//
	struct MaterialInstance
	{
		I_Material const* m_Material = nullptr;
		core::slot_map<Mesh> m_Meshes;
	};

	AssetPtr<rhi::ShaderData> m_Shader;
	core::slot_map<MaterialInstance> m_Materials;
};


} // namespace render
} // namespace et
