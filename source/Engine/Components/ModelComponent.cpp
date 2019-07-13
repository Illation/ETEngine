#include "stdafx.h"

#include "ModelComponent.hpp"

#include <iostream>

#include "TransformComponent.hpp"

#include <Engine/Content/ContentManager.hpp>

#include <Engine/Graphics/MeshFilter.hpp>
#include <Engine/Graphics/Material.hpp>
#include <Engine/Graphics/Frustum.hpp>

#include <Engine/SceneGraph/Entity.h>

#include <Engine/GraphicsHelper/RenderPipeline.hpp>
#include <Engine/GraphicsHelper/RenderState.hpp>
#include <Engine/GraphicsHelper/ShadowRenderer.hpp>

#include <Engine/Materials/NullMaterial.hpp>


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
			LOG("ModelComponent::UpdateMaterial> material is null", Warning);
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
		LOG("ModelComponent::Draw> material is null\n", LogLevel::Warning);
		return;
	}
	if (!(m_pMaterial->IsForwardRendered())) DrawCall();
}
void ModelComponent::DrawForward()
{
	if (m_pMaterial == nullptr)
	{
		LOG("ModelComponent::Draw> material is null\n", LogLevel::Warning);
		return;
	}
	if (m_pMaterial->IsForwardRendered()) DrawCall();
}
void ModelComponent::DrawCall()
{
	//Frustum culling
	switch (m_CullMode)
	{
	case CullMode::SPHERE:
	{
		auto filterSphere = m_pMeshFilter->GetBoundingSphere();
		vec3 scale = TRANSFORM->GetScale();
		float maxScale = std::max(scale.x, std::max(scale.y, scale.z));
		Sphere objSphere = Sphere(GetTransform()->GetPosition() + filterSphere->pos, filterSphere->radius*maxScale);
		if (CAMERA->GetFrustum()->ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
			return;
	}
	case CullMode::DISABLED:
		break;
	default:
		break;
	}
	//Get Vertex Object
	auto vO = m_pMeshFilter->GetVertexObject(m_pMaterial);
	STATE->BindVertexArray(vO.array);
	m_pMaterial->UploadVariables(m_pEntity->GetTransform()->GetWorld());
	// Draw 
	STATE->SetDepthEnabled(true);
	STATE->DrawElements(GL_TRIANGLES, (uint32)m_pMeshFilter->m_IndexCount, GL_UNSIGNED_INT, 0);
}

void ModelComponent::DrawShadow()
{
	auto nullMat = ShadowRenderer::GetInstance()->GetNullMaterial();
	mat4 matWVP = ShadowRenderer::GetInstance()->GetLightVP();
	auto vO = m_pMeshFilter->GetVertexObject(nullMat);
	STATE->BindVertexArray(vO.array);
	nullMat->UploadVariables(m_pEntity->GetTransform()->GetWorld(), matWVP);
	STATE->DrawElements(GL_TRIANGLES, (uint32)m_pMeshFilter->m_IndexCount, GL_UNSIGNED_INT, 0);
}

void ModelComponent::SetMaterial(Material* pMaterial)
{
	m_MaterialSet = true;
	m_pMaterial = pMaterial;
}