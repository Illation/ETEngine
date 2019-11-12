#pragma once

#include "RenderSceneFwd.h"
#include "Skybox.h"

#include <Engine/Graphics/PostProcessingSettings.h>


class MeshData;


namespace render {


//struct Planet
//{
//	Material* m_Material;
//	T_NodeId m_Transform;
//	::Planet m_Planet;
//};

//struct Atmosphere
//{
//	T_NodeId m_Transform;
//	::Atmosphere* m_Atmosphere;
//};

//struct Sprite
//{
//	T_NodeId m_Transform;
//	SpriteComponent* m_Sprite;
//};


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

public:
	typedef core::slot_map<MeshInstance>::id_type T_InstanceId;

	// functionality
	//-------------
	T_NodeId AddNode(mat4 const& transform);
	void UpdateNode(T_NodeId const node, mat4 const& transform);
	void RemoveNode(T_NodeId const node);

	T_InstanceId AddInstance(Material* const material, AssetPtr<MeshData> const mesh, T_NodeId const node);
	void RemoveInstance(T_InstanceId const instance);

	T_DirLightId AddDirectionalLight(DirectionalLight const& light);
	void UpdateDirectionalLight(T_DirLightId const lightId, DirectionalLight const& value);
	void RemoveDirectionalLight(T_DirLightId const lightId);

	void SetSkyboxMap(T_Hash const assetIdEnvMap);
	void SetSkyboxRoughness(float const value) { m_Skybox.m_Roughness = value; }

	// accessors
	//-------------
	core::slot_map<mat4> const& GetNodes() const { return m_Nodes; }
	core::slot_map<MaterialCollection> const& GetOpaqueRenderables() const { return m_OpaqueRenderables; }
	core::slot_map<DirectionalLight> const& GetDirectionalLights() const { return m_DirectionalLights; }

	Skybox const& GetSkybox() const { return m_Skybox; }

	PostProcessingSettings const& GetPostProcessingSettings() const { return m_PostProcessingSettings; }

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

	Skybox m_Skybox;
	//core::slot_map<MaterialCollection> m_ForwardRenderables;
	//core::slot_map<Atmosphere> m_Atmospheres;

	//core::slot_map<Sprite> m_Sprites;

	PostProcessingSettings m_PostProcessingSettings;
};


} // namespace render
