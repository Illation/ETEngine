#include "stdafx.h"
#include "RenderScene.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Graphics/EnvironmentMap.h>
#include <Engine/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <Engine/PlanetTech/StarField.h>


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
// Scene::AddInstance
//
// Adds a drawable mesh to the scene
//
Scene::T_InstanceId Scene::AddInstance(Material* const material, AssetPtr<MeshData> const mesh, T_NodeId const node)
{
	AssetPtr<ShaderData> const shader = material->GetShader();

	core::slot_map<MaterialCollection>& collectionGroup = material->IsForwardRendered() ? m_ForwardRenderables : m_OpaqueRenderables;
	
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
	newInstance.first->m_IsOpaque = !(material->IsForwardRendered());

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
			T_LightId const shadowId = m_DirectionalShadowData.insert(DirectionalShadowData(ivec2(1024, 1024) * 8)).second;
			
			ET_ASSERT(lightId == shadowId);
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
void Scene::SetSkyboxMap(T_Hash const assetIdEnvMap)
{
	if (assetIdEnvMap == 0u)
	{
		m_Skybox.m_EnvironmentMap = nullptr;
	}
	else
	{
		m_Skybox.m_EnvironmentMap = ResourceManager::Instance()->GetAssetData<EnvironmentMap>(assetIdEnvMap);
	}
}

//---------------------
// Scene::SetStarfield
//
void Scene::SetStarfield(T_Hash const assetId)
{
	if ((assetId == 0u) && (m_Starfield != nullptr))
	{
		delete m_Starfield;
		m_Starfield = nullptr;
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

//--------------------------
// Scene::AddMeshToMaterial
//
core::T_SlotId Scene::AddMeshToMaterial(MaterialCollection::MaterialInstance& material, AssetPtr<MeshData> const mesh, T_NodeId const node)
{
	T_ArrayLoc const vao = mesh->GetSurface(material.m_Material)->GetVertexArray();

	auto foundMeshIt = std::find_if(material.m_Meshes.begin(), material.m_Meshes.end(),
		[vao](MaterialCollection::Mesh const& matInst)
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
