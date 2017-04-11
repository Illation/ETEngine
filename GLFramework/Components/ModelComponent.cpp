#include "stdafx.hpp"
#include "ModelComponent.hpp"

#include "TransformComponent.hpp"

#include "..\Content\ContentManager.hpp"

#include "..\Graphics\MeshFilter.hpp"
#include "..\Graphics\Material.hpp"

#include "../SceneGraph/Entity.hpp"

#include <iostream>
#include "../GraphicsHelper/ShadowRenderer.hpp"
#include "../Game/Materials/NullMaterial.hpp"

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
		LOGGER::Log("ModelComponent::Draw> material is null\n", LogLevel::Warning);
		return;
	}
	if (!(m_pMaterial->IsForwardRendered())) DrawCall();
}
void ModelComponent::DrawForward()
{
	if (m_pMaterial == nullptr)
	{
		LOGGER::Log("ModelComponent::Draw> material is null\n", LogLevel::Warning);
		return;
	}
	if (m_pMaterial->IsForwardRendered()) DrawCall();
}
void ModelComponent::DrawCall()
{
	//Get Vertex Object
	auto vO = m_pMeshFilter->GetVertexObject(m_pMaterial);
	glBindVertexArray(vO.array);
	m_pMaterial->UploadVariables(m_pEntity->GetTransform()->GetWorld());
	// Draw 
	glEnable(GL_DEPTH_TEST);//should be done externally
	glDrawElementsInstanced(GL_TRIANGLES, m_pMeshFilter->m_IndexCount, GL_UNSIGNED_INT, 0, 1);
}

void ModelComponent::DrawShadow()
{
	auto nullMat = ShadowRenderer::GetInstance()->GetNullMaterial();
	glm::mat4 matWVP = ShadowRenderer::GetInstance()->GetLightWVP();
	auto vO = m_pMeshFilter->GetVertexObject(nullMat);
	glBindVertexArray(vO.array);
	nullMat->UploadVariables(m_pEntity->GetTransform()->GetWorld(), matWVP);
	glDrawElementsInstanced(GL_TRIANGLES, m_pMeshFilter->m_IndexCount, GL_UNSIGNED_INT, 0, 1);
}

void ModelComponent::SetMaterial(Material* pMaterial)
{
	m_MaterialSet = true;
	m_pMaterial = pMaterial;
}