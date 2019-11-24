#pragma once
#include <EtMath/Geometry.h>

#include <EtCore/Containers/slot_map.h>
#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsContext/GraphicsTypes.h>


class ShaderData;
class Material;


namespace render {


typedef core::slot_map<mat4>::id_type T_NodeId;


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
		T_ArrayLoc m_VAO;
		uint32 m_IndexCount;
		E_DataType m_IndexDataType;
		Sphere m_BoundingVolume;
		std::vector<T_NodeId> m_Instances;
	};

	//---------------------------------------
	// MaterialCollection::MaterialInstance
	//
	// Parameters for a shader and list of all meshes to be drawn with these parameters
	//
	struct MaterialInstance
	{
		Material* m_Material = nullptr;
		core::slot_map<Mesh> m_Meshes;
	};

	AssetPtr<ShaderData> m_Shader;
	core::slot_map<MaterialInstance> m_Materials;
};


} // namespace render
