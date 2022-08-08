#include "stdafx.h"
#include "RenderScene.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/GraphicsTypes/EnvironmentMap.h>
#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/PlanetTech/StarField.h>


namespace et {
namespace render {


//=======
// Scene
//=======


//----------------------
// Scene::AddNode
//
// Add a transformation to the scene
//
T_NodeId Scene::AddNode(mat4 const& transform)
{
	return m_Nodes.insert(mat4(transform)).second;
}

//----------------------
// Scene::UpdateNode
//
// Change the transformation of an existing node
//
void Scene::UpdateNode(T_NodeId const node, mat4 const& transform)
{
	m_Nodes[node] = transform;
}

//----------------------
// Scene::RemoveNode
//
// Remove a transformation from the scene
//
void Scene::RemoveNode(T_NodeId const node)
{
	m_Nodes.erase(node);
}

//----------------------
// Scene::AddCamera
//
// Add a default camera to the scene.. the camera parameters should be updated by accessing the non-const reference to it after
//
core::T_SlotId Scene::AddCamera()
{
	return m_Cameras.insert(Camera()).second;
}

//----------------------
// Scene::RemoveCamera
//
// Remove the camera from the scene
//
void Scene::RemoveCamera(core::T_SlotId const cameraId)
{
	m_Cameras.erase(cameraId);
}

//----------------------
// Scene::AddInstance
//
// Adds a drawable mesh to the scene
//
Scene::T_InstanceId Scene::AddInstance(I_Material const* const material, AssetPtr<MeshData> const mesh, T_NodeId const node)
{
	AssetPtr<rhi::ShaderData> const shader = material->GetBaseMaterial()->GetShaderAsset();

	bool const opaque = (material->GetBaseMaterial()->GetDrawType() == Material::E_DrawType::Opaque);
	core::slot_map<MaterialCollection>& collectionGroup = opaque ? m_OpaqueRenderables : m_ForwardRenderables;
	
	// find or create a collection for the shader
	auto foundCollectionIt = std::find_if(collectionGroup.begin(), collectionGroup.end(), [shader](MaterialCollection const& col)
		{
			return col.m_Shader == shader;
		});

	T_CollectionId collectionId = core::slot_map<MaterialCollection>::s_InvalidIndex;
	if (foundCollectionIt == collectionGroup.cend())
	{
		auto newCollection = collectionGroup.insert(MaterialCollection());
		foundCollectionIt = newCollection.first;
		collectionId = newCollection.second;
		foundCollectionIt->m_Shader = shader;
	}
	else
	{
		collectionId = collectionGroup.iterator_id(foundCollectionIt);
	}

	ET_ASSERT(collectionId != core::slot_map<MaterialCollection>::s_InvalidIndex);

	// find or create a materialInstance for the material
	auto foundMaterialIt = std::find_if(foundCollectionIt->m_Materials.begin(), foundCollectionIt->m_Materials.end(), 
		[material](MaterialCollection::MaterialInstance const& matInst)
		{
			return matInst.m_Material == material;
		});

	T_MaterialInstanceId materialId = core::slot_map<MaterialCollection::MaterialInstance>::s_InvalidIndex;
	if (foundMaterialIt == foundCollectionIt->m_Materials.cend())
	{
		auto newMaterial = foundCollectionIt->m_Materials.insert(MaterialCollection::MaterialInstance());
		foundMaterialIt = newMaterial.first;
		materialId = newMaterial.second;
		foundMaterialIt->m_Material = material;
	}
	else
	{
		materialId = foundCollectionIt->m_Materials.iterator_id(foundMaterialIt);
	}

	ET_ASSERT(materialId != core::slot_map<MaterialCollection::MaterialInstance>::s_InvalidIndex);

	// find or create a mesh in the material instance
	T_MeshId meshId = AddMeshToMaterial(*foundMaterialIt, mesh, node);

	// also make the mesh cast a shadow
	if (m_ShadowCasters.m_Material == nullptr)
	{
		m_ShadowCasters.m_Material = RenderingSystems::Instance()->GetNullMaterial();
	}
	T_MeshId casterId = AddMeshToMaterial(m_ShadowCasters, mesh, node);

	// link the instance data to its own ID
	auto newInstance = m_Instances.insert(MeshInstance());

	newInstance.first->m_Collection = collectionId;
	newInstance.first->m_Material = materialId;
	newInstance.first->m_Mesh = meshId;
	newInstance.first->m_ShadowCaster = casterId;
	newInstance.first->m_Transform = node;
	newInstance.first->m_IsOpaque = opaque;

	return newInstance.second;
}

//----------------------
// Scene::RemoveInstance
//
void Scene::RemoveInstance(T_InstanceId const instance)
{
	MeshInstance const& inst = m_Instances[instance];

	core::slot_map<MaterialCollection>& collectionGroup = inst.m_IsOpaque ? m_OpaqueRenderables : m_ForwardRenderables;

	MaterialCollection& collection = collectionGroup[inst.m_Collection];
	MaterialCollection::MaterialInstance& material = collection.m_Materials[inst.m_Material];
	RemoveMeshFromMaterial(m_ShadowCasters, inst.m_ShadowCaster, inst.m_Transform);
	RemoveMeshFromMaterial(material, inst.m_Mesh, inst.m_Transform);
	if (material.m_Meshes.size() == 0u)
	{
		if (collection.m_Materials.size() == 1u)
		{
			if (collectionGroup.size() == 1u)
			{
				collectionGroup.clear();
			}
			else
			{
				collection.m_Materials.erase(inst.m_Material);
			}
		}
		else
		{
			collection.m_Materials.erase(inst.m_Material);
		}
	}
	
	m_Instances.erase(instance);
}

//----------------------
// Scene::AddDirectionalLight
//
T_LightId Scene::AddLight(vec3 const& color, T_NodeId const node, bool const isDirectional, bool const hasShadow)
{
	ET_ASSERT(hasShadow ? isDirectional : true, "point lights don't support shadow maps");

	Light light;
	light.m_Color = color;
	light.m_NodeId = node;

	T_LightId lightId;
	if (isDirectional)
	{
		if (hasShadow)
		{
			lightId = m_DirectionalLightsShaded.insert(Light(light)).second;

			auto shadowData = m_DirectionalShadowData.insert(DirectionalShadowData());
			shadowData.first->Init(ivec2(1024, 1024) * 8);
			
			ET_ASSERT(lightId == shadowData.second);
		}
		else
		{
			lightId = m_DirectionalLights.insert(Light(light)).second;
		}
	}
	else
	{
		lightId = m_PointLights.insert(Light(light)).second;
	}

	LightInstance instance;
	instance.m_SlotId = lightId;
	instance.m_IsDirectional = isDirectional;
	instance.m_HasShadow = hasShadow;

	return m_Lights.insert(LightInstance(instance)).second;
}

//----------------------
// Scene::UpdateDirectionalLight
//
void Scene::UpdateLightColor(T_LightId const lightId, vec3 const& value)
{
	LightInstance const& instance = m_Lights[lightId];

	if (instance.m_IsDirectional)
	{
		if (instance.m_HasShadow)
		{
			m_DirectionalLightsShaded[instance.m_SlotId].m_Color = value;
		}
		else
		{
			m_DirectionalLights[instance.m_SlotId].m_Color = value;
		}
	}
	else
	{
		m_PointLights[instance.m_SlotId].m_Color = value;
	}
}

//----------------------
// Scene::RemoveDirectionalLight
//
void Scene::RemoveLight(T_LightId const lightId)
{
	LightInstance const& instance = m_Lights[lightId];

	if (instance.m_IsDirectional)
	{
		if (instance.m_HasShadow)
		{
			m_DirectionalLightsShaded.erase(instance.m_SlotId);

			m_DirectionalShadowData[instance.m_SlotId].Destroy();
			m_DirectionalShadowData.erase(instance.m_SlotId);
		}
		else
		{
			m_DirectionalLights.erase(instance.m_SlotId);
		}
	}
	else
	{
		m_PointLights.erase(instance.m_SlotId);
	}

	m_Lights.erase(lightId);
}

//----------------------
// Scene::SetSkyboxMap
//
void Scene::SetSkyboxMap(core::HashString const assetIdEnvMap)
{
	if (assetIdEnvMap.IsEmpty())
	{
		m_Skybox.m_EnvironmentMap = nullptr;
	}
	else
	{
		m_Skybox.m_EnvironmentMap = core::ResourceManager::Instance()->GetAssetData<EnvironmentMap>(assetIdEnvMap);
	}
}

//---------------------
// Scene::SetStarfield
//
void Scene::SetStarfield(core::HashString const assetId)
{
	if (assetId.IsEmpty())
	{
		if (m_Starfield != nullptr)
		{
			delete m_Starfield;
			m_Starfield = nullptr;
		}
	}
	else
	{
		if (m_Starfield != nullptr)
		{
			delete m_Starfield;
		}

		m_Starfield = new StarField(assetId);
	}
}

//------------------
// Scene::AddPlanet
//
core::T_SlotId Scene::AddPlanet(PlanetParams const& params, T_NodeId const node)
{
	auto newPlanet = m_Terrains.insert(Planet());
	newPlanet.first->Init(params, node);
	return newPlanet.second;
}

//---------------------
// Scene::RemovePlanet
//
void Scene::RemovePlanet(core::T_SlotId const planetId)
{
	m_Terrains.erase(planetId);
}

//----------------------
// Scene::AddAtmosphere
//
// Create an instance of an atmosphere, and if there is no render data for the inst, create new rendering data as well
//
core::T_SlotId Scene::AddAtmosphere(AtmosphereInstance const& inst)
{
	auto newInst = m_AtmosphereInstances.insert(AtmosphereInstance(inst));

	auto const foundAtmo = std::find_if(m_Atmospheres.begin(), m_Atmospheres.end(), [&inst](std::pair<Atmosphere, uint8> const& atmo)
		{
			return atmo.first.GetId() == inst.atmosphereId;
		});

	if (foundAtmo != m_Atmospheres.cend())
	{
		foundAtmo->second++;
	}
	else
	{
		m_Atmospheres.emplace_back(Atmosphere(), 1u);
		std::prev(m_Atmospheres.end())->first.Initialize(inst.atmosphereId);
	}

	return newInst.second;
}

//------------------------------
// Scene::UpdateAtmosphereLight
//
void Scene::UpdateAtmosphereLight(core::T_SlotId const atmoId, T_LightId const lightId)
{
	m_AtmosphereInstances[atmoId].lightId = lightId;
}

//-------------------------
// Scene::RemoveAtmosphere
//
// Remove an instance of an atmosphere, and if it was the last instance of a rendering data set, also remove the rendering data
//
void Scene::RemoveAtmosphere(core::T_SlotId const atmoId)
{
	AtmosphereInstance const& inst = m_AtmosphereInstances[atmoId];

	auto const foundAtmo = std::find_if(m_Atmospheres.begin(), m_Atmospheres.end(), [&inst](std::pair<Atmosphere, uint8> const& atmo)
		{
			return atmo.first.GetId() == inst.atmosphereId;
		});
	ET_ASSERT(foundAtmo != m_Atmospheres.cend());

	if (foundAtmo->second == 1u)
	{
		core::RemoveSwap(m_Atmospheres, foundAtmo);
	}
	else
	{
		foundAtmo->second--;
	}

	m_AtmosphereInstances.erase(atmoId);
}

//---------------------
// Scene::AddExtension
//
// Takes ownership
//
void Scene::AddExtension(UniquePtr<I_SceneExtension>& ext)
{
	ET_ASSERT(GetExtension(ext->GetId()) == nullptr, "Extension was already added!");

	m_Extensions.emplace_back(std::move(ext));
}

//------------------------
// Scene::ClearExtensions
//
void Scene::ClearExtensions()
{
	m_Extensions.clear();
}

//-----------------
// Scene::GetLight
//
Camera& Scene::GetCamera(core::T_SlotId const cameraId)
{
	return m_Cameras[cameraId];
}

//-----------------
// Scene::GetLight
//
Light const& Scene::GetLight(T_LightId const lightId) const
{
	LightInstance const& instance = m_Lights[lightId];

	if (instance.m_IsDirectional)
	{
		if (instance.m_HasShadow)
		{
			return m_DirectionalLightsShaded[instance.m_SlotId];
		}
		else
		{
			return m_DirectionalLights[instance.m_SlotId];
		}
	}
	else
	{
		return m_PointLights[instance.m_SlotId];
	}
}

//----------------------
// Scene::GetAtmosphere
//
render::Atmosphere const& Scene::GetAtmosphere(core::HashString const atmoId) const
{
	auto const foundAtmo = std::find_if(m_Atmospheres.cbegin(), m_Atmospheres.cbegin(), [atmoId](std::pair<Atmosphere, uint8> const& atmo)
	{
		return atmo.first.GetId() == atmoId;
	});

	ET_ASSERT(foundAtmo != m_Atmospheres.cend());

	return foundAtmo->first;
}

//---------------------
// Scene::GetExtension
//
// Get an extension, if we have it, else null
//
render::I_SceneExtension* Scene::GetExtension(core::HashString const extensionId) const
{
	auto foundIt = std::find_if(m_Extensions.cbegin(), m_Extensions.cend(), [extensionId](UniquePtr<I_SceneExtension> const& ext)
		{
			return ext->GetId() == extensionId;
		});

	if (foundIt == m_Extensions.cend())
	{
		return nullptr;
	}

	return (*foundIt).Get();
}

//--------------------------
// Scene::AddMeshToMaterial
//
core::T_SlotId Scene::AddMeshToMaterial(MaterialCollection::MaterialInstance& material, AssetPtr<MeshData> const mesh, T_NodeId const node)
{
	rhi::T_ArrayLoc const vao = mesh->GetSurface(material.m_Material->GetBaseMaterial())->GetVertexArray();

	auto foundMeshIt = std::find_if(material.m_Meshes.begin(), material.m_Meshes.end(), [vao](MaterialCollection::Mesh const& matInst)
		{
			return matInst.m_VAO == vao;
		});

	T_MaterialInstanceId meshId = core::slot_map<MaterialCollection::MaterialInstance>::s_InvalidIndex;
	if (foundMeshIt == material.m_Meshes.cend())
	{
		auto newMesh = material.m_Meshes.insert(MaterialCollection::Mesh());

		foundMeshIt = newMesh.first;
		meshId = newMesh.second;

		foundMeshIt->m_VAO = vao;
		foundMeshIt->m_IndexCount = static_cast<uint32>(mesh->GetIndexCount());
		foundMeshIt->m_IndexDataType = mesh->GetIndexDataType();
		foundMeshIt->m_BoundingVolume = mesh->GetBoundingSphere();
	}
	else
	{
		meshId = material.m_Meshes.iterator_id(foundMeshIt);
	}

	ET_ASSERT(meshId != core::slot_map<MaterialCollection::Mesh>::s_InvalidIndex);

	foundMeshIt->m_Instances.emplace_back(node);

	return meshId;
}

//-------------------------------
// Scene::RemoveMeshFromMaterial
//
void Scene::RemoveMeshFromMaterial(MaterialCollection::MaterialInstance& material, T_MeshId meshId, T_NodeId node)
{
	MaterialCollection::Mesh& mesh = material.m_Meshes[meshId];
	if (mesh.m_Instances.size() == 1u)
	{			
		material.m_Meshes.erase(meshId);
	}
	else
	{
		auto foundTransform = std::find(mesh.m_Instances.begin(), mesh.m_Instances.end(), node);
		ET_ASSERT(foundTransform != mesh.m_Instances.cend());

		std::iter_swap(foundTransform, std::prev(mesh.m_Instances.end()));
		mesh.m_Instances.pop_back();
	}
}


} // namespace render
} // namespace et
