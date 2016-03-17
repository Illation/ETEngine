#include "stdafx.hpp"
#include "ModelComponent.hpp"

#include "TransformComponent.hpp"

#include "..\Content\ContentManager.hpp"

#include "..\Graphics\MeshFilter.hpp"
#include "..\Graphics\Material.hpp"

#include "../SceneGraph/Entity.hpp"

#include <iostream>

ModelComponent::ModelComponent(std::string assetFile):
	m_AssetFile(assetFile)
{
}
ModelComponent::~ModelComponent()
{
}

void ModelComponent::Initialize()
{
	m_pMeshFilter = ContentManager::Load<MeshFilter>(m_AssetFile);
	UpdateMaterial();
}

void ModelComponent::UpdateMaterial()
{
	if (m_MaterialSet)
	{
		m_MaterialSet = false;
		if (m_pMaterial == nullptr)
		{
			std::cout << "ModelComponent::UpdateMaterial> material is null\n";
			return;
		}
		m_pMaterial->Initialize();
		m_pMeshFilter->BuildVertexBuffer(m_pMaterial);
	}
}

void ModelComponent::Update()
{
	UpdateMaterial();
}

void ModelComponent::Draw()
{
	if (m_pMaterial == nullptr)
	{
		std::cout << "ModelComponent::Draw> material is null\n";
		return;
	}
	//Get Vertex Object
	auto vO = m_pMeshFilter->GetVertexObject(m_pMaterial);
	glBindVertexArray(vO.array);
	m_pMaterial->UploadVariables(m_pEntity->GetTransform()->GetWorld());
	// Draw 
	glEnable(GL_DEPTH_TEST);
	glDrawElements(GL_TRIANGLES, m_pMeshFilter->m_IndexCount, GL_UNSIGNED_INT, 0);
}

void ModelComponent::SetMaterial(Material* pMaterial)
{
	m_MaterialSet = true;
	m_pMaterial = pMaterial;
}