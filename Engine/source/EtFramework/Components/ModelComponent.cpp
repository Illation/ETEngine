#include "stdafx.h"
#include "ModelComponent.h"

#include "TransformComponent.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
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
// ModelComponent::Init
//
// Loads the mesh data
//
void ModelComponent::Init()
{
	// Load mesh
	m_Mesh = ResourceManager::Instance()->GetAssetData<MeshData>(m_MeshId);

	// Load material
	m_Material = ResourceManager::Instance()->GetAssetData<render::Material>(m_MaterialId, false);
	if (m_Material == nullptr)
	{
		m_Material = ResourceManager::Instance()->GetAssetData<render::MaterialInstance>(m_MaterialId);
	}

	render::I_Material const* const mat = m_Material.get_as<render::I_Material>();

	// register
	m_InstanceId = SceneManager::GetInstance()->GetRenderScene().AddInstance(mat, m_Mesh, GetTransform()->GetNodeId());
}

//---------------------------------
// ModelComponent::Deinit
//
void ModelComponent::Deinit()
{
	SceneManager::GetInstance()->GetRenderScene().RemoveInstance(m_InstanceId);
}
