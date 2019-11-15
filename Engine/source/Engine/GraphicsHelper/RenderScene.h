#pragma once

#include "RenderSceneFwd.h"
#include "Skybox.h"
#include "Sprite.h"

#include <Engine/PlanetTech/Atmosphere.h>
#include <Engine/PlanetTech/Planet.h>

#include <Engine/Graphics/PostProcessingSettings.h>
#include <Engine/SceneRendering/ShadowRenderer.h>


class MeshData;
class StarField;


namespace render {


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

	T_InstanceId AddInstance(Material* const material, AssetPtr<MeshData> const mesh, T_NodeId const node);
	void RemoveInstance(T_InstanceId const instance);

	T_LightId AddLight(vec3 const& color, T_NodeId const node, bool const isDirectional, bool const hasShadow);
	void UpdateLightColor(T_LightId const lightId, vec3 const& value);
	void RemoveLight(T_LightId const lightId);

	void SetSkyboxMap(T_Hash const assetIdEnvMap);
	void SetSkyboxRoughness(float const value) { m_Skybox.m_Roughness = value; }

	void SetStarfield(T_Hash const assetId);

	core::T_SlotId AddPlanet(PlanetParams const& params, T_NodeId const node);
	void RemovePlanet(core::T_SlotId const planetId);

	core::T_SlotId AddAtmosphere(AtmosphereInstance const& inst);
	void UpdateAtmosphereLight(core::T_SlotId const atmoId, T_LightId const lightId);
	void RemoveAtmosphere(core::T_SlotId const atmoId);

	core::T_SlotId AddSprite(T_Hash const textureId, T_NodeId const node, vec2 const pivot, vec4 const& color);
	void UpdateSpriteTexture(core::T_SlotId const spriteId, T_Hash const textureId);
	void UpdateSpritePivot(core::T_SlotId const spriteId, vec2 const pivot);
	void UpdateSpriteColor(core::T_SlotId const spriteId, vec4 const& color);
	void RemoveSprite(core::T_SlotId const spriteId);


	// accessors
	//-------------
	core::slot_map<mat4> const& GetNodes() const { return m_Nodes; }

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
	Atmosphere const& GetAtmosphere(T_Hash const atmoId) const;

	core::slot_map<Sprite> const& GetSprites() const { return m_Sprites; }

	PostProcessingSettings const& GetPostProcessingSettings() const { return m_PostProcessingSettings; }


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

	core::slot_map<Sprite> m_Sprites;

	PostProcessingSettings m_PostProcessingSettings;
};


} // namespace render
