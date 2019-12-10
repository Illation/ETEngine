#include "stdafx.h"
#include "ModelComponent.h"

#include <iostream>

#include "TransformComponent.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/GraphicsTypes/Material.h>
#include <EtRendering/MaterialSystem/MaterialInstance.h>
#include <EtRendering/MaterialSystem/MaterialData.h>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/SceneGraph/SceneManager.h>


//=================
// Model Component
//=================


//---------------------------------
// ModelComponent::c-tor
//
// default constructor sets the asset ID
//
ModelComponent::ModelComponent(T_Hash const meshId, T_Hash const materialId)
	: m_MeshId(meshId)
	, m_MaterialId(materialId)
{ }

//---------------------------------
// ModelComponent::SetMaterial
//
// Updates the material and marks it as dirty
//
void ModelComponent::SetMaterial(Material* pMaterial)
{
	m_MaterialSet = true;
	m_Material = pMaterial;
}

//---------------------------------
// ModelComponent::Init
//
// Loads the mesh data
//
void ModelComponent::Init()
{
	m_Mesh = ResourceManager::Instance()->GetAssetData<MeshData>(m_MeshId);

	if (m_MaterialId != 0u)
	{
		m_MaterialAsset = ResourceManager::Instance()->GetAssetData<render::Material>(m_MaterialId, false);
		if (m_MaterialAsset == nullptr)
		{
			m_MaterialAsset = ResourceManager::Instance()->GetAssetData<render::MaterialInstance>(m_MaterialId);
		}
	}


	UpdateMaterial();

	m_InstanceId = SceneManager::GetInstance()->GetRenderScene().AddInstance(m_Material, m_Mesh, GetTransform()->GetNodeId());
}

//---------------------------------
// ModelComponent::Deinit
//
void ModelComponent::Deinit()
{
	SceneManager::GetInstance()->GetRenderScene().RemoveInstance(m_InstanceId);
}

//---------------------------------
// ModelComponent::UpdateMaterial
//
// Links the mesh to the material
//
void ModelComponent::UpdateMaterial()
{
	if (m_MaterialSet)
	{
		m_MaterialSet = false;
		if (m_Material == nullptr)
		{
			LOG("ModelComponent::UpdateMaterial> material is null", Warning);
			return;
		}

		m_Material->Initialize();
		m_Mesh->GetSurface(m_Material); // make sure we have a mesh surface cached for our material
	}
}

//---------------------------------
// ModelComponent::Update
//
// Update the material if it changed
//
void ModelComponent::Update()
{
	UpdateMaterial();
}
