#include "stdafx.h"
#include "RenderScene.h"

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/Material.h>



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
	
	// find or create a collection for the shader
	auto foundCollectionIt = std::find_if(m_OpaqueRenderables.begin(), m_OpaqueRenderables.end(), [shader](MaterialCollection const& col)
		{
			return col.m_Shader == shader;
		});

	T_CollectionId collectionId = core::slot_map<MaterialCollection>::s_InvalidIndex;
	if (foundCollectionIt == m_OpaqueRenderables.cend())
	{
		auto newCollection = m_OpaqueRenderables.insert(MaterialCollection());
		foundCollectionIt = newCollection.first;
		collectionId = newCollection.second;
		foundCollectionIt->m_Shader = shader;
	}
	else
	{
		collectionId = m_OpaqueRenderables.iterator_id(foundCollectionIt);
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
	T_ArrayLoc const vao = mesh->GetSurface(material)->GetVertexArray();

	auto foundMeshIt = std::find_if(foundMaterialIt->m_Meshes.begin(), foundMaterialIt->m_Meshes.end(),
		[vao](MaterialCollection::Mesh const& matInst)
		{
			return matInst.m_VAO == vao;
		});

	T_MaterialInstanceId meshId = core::slot_map<MaterialCollection::MaterialInstance>::s_InvalidIndex;
	if (foundMeshIt == foundMaterialIt->m_Meshes.cend())
	{
		auto newMesh = foundMaterialIt->m_Meshes.insert(MaterialCollection::Mesh());

		foundMeshIt = newMesh.first;
		meshId = newMesh.second;

		foundMeshIt->m_VAO = vao;
		foundMeshIt->m_BoundingVolume = mesh->GetBoundingSphere();
	}
	else
	{
		meshId = foundMaterialIt->m_Meshes.iterator_id(foundMeshIt);
	}

	ET_ASSERT(meshId != core::slot_map<MaterialCollection::Mesh>::s_InvalidIndex);

	foundMeshIt->m_Instances.emplace_back(node);

	// link the instance data to its own ID
	auto newInstance = m_Instances.insert(MeshInstance());

	newInstance.first->m_Collection = collectionId;
	newInstance.first->m_Material = materialId;
	newInstance.first->m_Mesh = meshId;
	newInstance.first->m_Transform = node;
	newInstance.first->m_IsOpaque = true;

	return newInstance.second;
}

//----------------------
// Scene::RemoveInstance
//
void Scene::RemoveInstance(T_InstanceId const instance)
{
	MeshInstance const& inst = m_Instances[instance];

	MaterialCollection& collection = m_OpaqueRenderables[inst.m_Collection];
	MaterialCollection::MaterialInstance& material = collection.m_Materials[inst.m_Material];
	MaterialCollection::Mesh& mesh = material.m_Meshes[inst.m_Mesh];
	if (mesh.m_Instances.size() == 1u)
	{
		if (material.m_Meshes.size() == 1u)
		{
			if (collection.m_Materials.size() == 1u)
			{
				if (m_OpaqueRenderables.size() == 1u)
				{
					m_OpaqueRenderables.clear();
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
		else
		{
			material.m_Meshes.erase(inst.m_Mesh);
		}
	}
	else
	{
		auto foundTransform = std::find(mesh.m_Instances.begin(), mesh.m_Instances.end(), inst.m_Transform);
		ET_ASSERT(foundTransform != mesh.m_Instances.cend());

		std::iter_swap(foundTransform, std::prev(mesh.m_Instances.end()));
		mesh.m_Instances.pop_back();
	}

	m_Instances.erase(instance);
}

//----------------------
// Scene::AddDirectionalLight
//
T_DirLightId Scene::AddDirectionalLight(DirectionalLight const& light)
{
	return m_DirectionalLights.insert(DirectionalLight(light)).second;
}

//----------------------
// Scene::UpdateDirectionalLight
//
void Scene::UpdateDirectionalLight(T_DirLightId const lightId, DirectionalLight const& value)
{
	m_DirectionalLights[lightId] = value;
}

//----------------------
// Scene::RemoveDirectionalLight
//
void Scene::RemoveDirectionalLight(T_DirLightId const lightId)
{
	m_DirectionalLights.erase(lightId);
}


} // namespace render
