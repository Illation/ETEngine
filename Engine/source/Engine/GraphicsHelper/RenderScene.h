#pragma once
#include <EtMath/Geometry.h>

#include <EtCore/Containers/slot_map.h>
#include <EtCore/Content/AssetPointer.h>

#include <Engine/GraphicsContext/GraphicsTypes.h>


class ShaderData;
class MeshData;
class Material;


typedef core::slot_map<mat4>::id_type T_NodeId;


namespace render {


//struct Planet
//{
//	Material* m_Material;
//	T_NodeId m_Transform;
//	::Planet m_Planet;
//};

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
		Material* m_Material;
		core::slot_map<Mesh> m_Meshes;
	};

	AssetPtr<ShaderData> m_Shader;
	core::slot_map<MaterialInstance> m_Materials;
};

//struct PointLight
//{
//	vec3 m_Color;
//	float m_Brightness;
//	vec3 m_Position;
//	float m_Radius;
//};
struct DirectionalLight
{
	vec3 m_Color;
	float m_Brightness;
	vec3 m_Direction;
};
//struct ShadedDirectionalLight
//{
//	vec3 m_Color;
//	float m_Brightness;
//	vec3 m_Direction;
//	DirectionalShadowData m_ShadowData;
//};
//
//struct Skybox
//{
//	T_ArrayLoc m_VAO;
//	AssetPtr<ShaderData> m_Shader;
//	float m_Roughness;
//};
//
//struct Atmosphere
//{
//	T_NodeId m_Transform;
//	::Atmosphere* m_Atmosphere;
//};
//
//struct Sprite
//{
//	T_NodeId m_Transform;
//	SpriteComponent* m_Sprite;
//};


typedef core::slot_map<MaterialCollection::Mesh>::id_type T_MeshId;
typedef core::slot_map<MaterialCollection::MaterialInstance>::id_type T_MaterialInstanceId;
typedef core::slot_map<MaterialCollection>::id_type T_CollectionId;

typedef core::slot_map<DirectionalLight>::id_type T_DirLightId;


//----------------------
// Scene
//
// Structure to arrange and update data in a practical way for 3D rendering
//
class Scene
{
	// definitions
	//-------------

	//----------------------
	// MeshInstance
	//
	// Allows mapping of internal data combinations forming a mesh to be mapped to a single ID
	//
	struct MeshInstance
	{
		T_CollectionId m_Collection;
		T_MaterialInstanceId m_Material;
		T_MeshId m_Mesh;
		T_NodeId m_Transform;
		bool m_IsOpaque;
	};

	typedef core::slot_map<MeshInstance>::id_type T_InstanceId;

	// functionality
	//-------------
public:
	T_NodeId AddNode(mat4 const& transform);
	void UpdateNode(T_NodeId const node, mat4 const& transform);
	void RemoveNode(T_NodeId const node);

	T_InstanceId AddInstance(Material* const material, MeshData* const mesh, T_NodeId const node);
	void RemoveInstance(T_InstanceId const instance);

	T_DirLightId AddDirectionalLight(DirectionalLight const& light);
	void UpdateDirectionalLight(T_DirLightId const lightId, DirectionalLight const& value);
	void RemoveDirectionalLight(T_DirLightId const lightId);


	// Data
	///////

private:

	// mapping
	core::slot_map<MeshInstance> m_Instances;

	// renderable
	core::slot_map<mat4> m_Nodes;

	//core::slot_map<Planet> m_Terrains;
	core::slot_map<MaterialCollection> m_OpaqueRenderables;

	//core::slot_map<PointLight> m_PointLights;
	core::slot_map<DirectionalLight> m_DirectionalLights;
	//core::slot_map<ShadedDirectionalLight> m_ShadedDirectional;

	//Skybox m_Skybox;
	//core::slot_map<MaterialCollection> m_ForwardRenderables;
	//core::slot_map<Atmosphere> m_Atmospheres;

	//core::slot_map<Sprite> m_Sprites;
};


} // namespace render
