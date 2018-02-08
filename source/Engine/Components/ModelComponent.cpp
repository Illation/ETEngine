#include "stdafx.hpp"
#include "ModelComponent.hpp"

#include "TransformComponent.hpp"

#include "../Content/ContentManager.hpp"

#include "../Graphics/MeshFilter.hpp"
#include "../Graphics/Material.hpp"

#include "../SceneGraph/Entity.hpp"

#include <iostream>
#include "../GraphicsHelper/ShadowRenderer.hpp"
#include "../Materials/NullMaterial.hpp"
#include "../Graphics/Frustum.hpp"
#include "../GraphicsHelper/RenderPipeline.hpp"
#include "../GraphicsHelper/RenderState.hpp"

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
		float maxScale = max(scale.x, max(scale.y, scale.z));
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
	glDrawElementsInstanced(GL_TRIANGLES, (uint32)m_pMeshFilter->m_IndexCount, GL_UNSIGNED_INT, 0, 1);
	PERFORMANCE->m_DrawCalls++;
}

void ModelComponent::DrawShadow()
{
	auto nullMat = ShadowRenderer::GetInstance()->GetNullMaterial();
	mat4 matWVP = ShadowRenderer::GetInstance()->GetLightVP();
	auto vO = m_pMeshFilter->GetVertexObject(nullMat);
	STATE->BindVertexArray(vO.array);
	nullMat->UploadVariables(m_pEntity->GetTransform()->GetWorld(), matWVP);
	glDrawElementsInstanced(GL_TRIANGLES, (uint32)m_pMeshFilter->m_IndexCount, GL_UNSIGNED_INT, 0, 1);
	PERFORMANCE->m_DrawCalls++;
}

void ModelComponent::SetMaterial(Material* pMaterial)
{
	m_MaterialSet = true;
	m_pMaterial = pMaterial;
}