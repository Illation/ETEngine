#pragma once
#include "RenderSceneFwd.h"
#include "Skybox.h"

#include <EtRendering/Extensions/SceneExtension.h>
#include <EtRendering/PlanetTech/Atmosphere.h>
#include <EtRendering/PlanetTech/Planet.h>
#include <EtRendering/GraphicsTypes/PostProcessingSettings.h>
#include <EtRendering/GraphicsTypes/DirectionalShadowData.h>
#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/GraphicsTypes/Camera.h>


namespace et {
namespace render {
	

class StarField;


//----------------------
// Scene
//
// Structure to arrange and update data in a practical way for 3D rendering
//
class Scene final
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
		T_MeshId m_ShadowCaster;
		T_NodeId m_Transform;
		bool m_IsOpaque;
	};

	//----------------------
	// LightInstance
	//
	// Allows mapping of internal data combinations forming a mesh to be mapped to a single ID
	//
	struct LightInstance
	{
		T_LightId m_SlotId;
		bool m_IsDirectional;
		bool m_HasShadow;
	};

public:
	typedef core::slot_map<MeshInstance>::id_type T_InstanceId;

	// functionality
	//-------------
	T_NodeId AddNode(mat4 const& transform);
	void UpdateNode(T_NodeId const node, mat4 const& transform);
	void RemoveNode(T_NodeId const node);

	core::T_SlotId AddCamera();
	void RemoveCamera(core::T_SlotId const cameraId);

	T_InstanceId AddInstance(I_Material const* const material, AssetPtr<MeshData> const mesh, T_NodeId const node);
	void RemoveInstance(T_InstanceId const instance);

	T_LightId AddLight(vec3 const& color, T_NodeId const node, bool const isDirectional, bool const hasShadow);
	void UpdateLightColor(T_LightId const lightId, vec3 const& value);
	void RemoveLight(T_LightId const lightId);

	void SetSkyboxMap(core::HashString const assetIdEnvMap);
	void SetSkyboxRoughness(float const value) { m_Skybox.m_Roughness = value; }

	void SetStarfield(core::HashString const assetId);

	void SetPostProcessingSettings(PostProcessingSettings const& settings) { m_PostProcessingSettings = settings; }

	core::T_SlotId AddPlanet(PlanetParams const& params, T_NodeId const node);
	void RemovePlanet(core::T_SlotId const planetId);

	core::T_SlotId AddAtmosphere(AtmosphereInstance const& inst);
	void UpdateAtmosphereLight(core::T_SlotId const atmoId, T_LightId const lightId);
	void RemoveAtmosphere(core::T_SlotId const atmoId);

	void AddExtension(UniquePtr<I_SceneExtension>& ext);
	void ClearExtensions();

	// accessors
	//-------------
	core::slot_map<mat4> const& GetNodes() const { return m_Nodes; }

	Camera& GetCamera(core::T_SlotId const cameraId);
	core::slot_map<Camera> const& GetCameras() const { return m_Cameras; }

	core::slot_map<Planet> const& GetTerrains() const { return m_Terrains; }
	core::slot_map<Planet>& GetTerrains() { return m_Terrains; }

	core::slot_map<MaterialCollection> const& GetOpaqueRenderables() const { return m_OpaqueRenderables; }
	core::slot_map<MaterialCollection> const& GetForwardRenderables() const { return m_ForwardRenderables; }

	core::slot_map<Light> const& GetPointLights() const { return m_PointLights; }
	core::slot_map<Light> const& GetDirectionalLights() const { return m_DirectionalLights; }
	core::slot_map<Light> const& GetDirectionalLightsShaded() const { return m_DirectionalLightsShaded; }

	Light const& GetLight(T_LightId const lightId) const;

	core::slot_map<DirectionalShadowData>& GetDirectionalShadowData() { return m_DirectionalShadowData; }
	core::slot_map<DirectionalShadowData> const& GetDirectionalShadowData() const { return m_DirectionalShadowData; }

	MaterialCollection::MaterialInstance const& GetShadowCasters() const { return m_ShadowCasters; }

	Skybox const& GetSkybox() const { return m_Skybox; }
	StarField const* GetStarfield() const { return m_Starfield; }

	core::slot_map<AtmosphereInstance> const& GetAtmosphereInstances() const { return m_AtmosphereInstances; }
	Atmosphere const& GetAtmosphere(core::HashString const atmoId) const;

	PostProcessingSettings const& GetPostProcessingSettings() const { return m_PostProcessingSettings; }

	I_SceneExtension* GetExtension(core::HashString const extensionId) const;


	// utility
	//---------
private:
	core::T_SlotId AddMeshToMaterial(MaterialCollection::MaterialInstance& material, AssetPtr<MeshData> const mesh, T_NodeId const node);
	void RemoveMeshFromMaterial(MaterialCollection::MaterialInstance& material, T_MeshId meshId, T_NodeId node);


	// Data
	///////

	// internal mapping
	//------------------
	core::slot_map<MeshInstance> m_Instances;
	core::slot_map<LightInstance> m_Lights;

	// accessible render data
	//------------------------
	core::slot_map<mat4> m_Nodes;

	core::slot_map<Camera> m_Cameras;

	core::slot_map<Planet> m_Terrains;
	core::slot_map<MaterialCollection> m_OpaqueRenderables;

	core::slot_map<Light> m_PointLights;
	core::slot_map<Light> m_DirectionalLights;

	core::slot_map<Light> m_DirectionalLightsShaded;
	core::slot_map<DirectionalShadowData> m_DirectionalShadowData;

	MaterialCollection::MaterialInstance m_ShadowCasters;

	Skybox m_Skybox;
	StarField* m_Starfield = nullptr;
	core::slot_map<MaterialCollection> m_ForwardRenderables;
	core::slot_map<AtmosphereInstance> m_AtmosphereInstances; // map into atmospheres - #todo: make atmosphere lists contain their instances
	std::vector<std::pair<Atmosphere, uint8>> m_Atmospheres; // < renderable atmosphere | refcount >

	PostProcessingSettings m_PostProcessingSettings;

	std::vector<UniquePtr<I_SceneExtension>> m_Extensions;
};


} // namespace render
} // namespace et
