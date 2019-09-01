#include "stdafx.h"
#include "ModelComponent.h"

#include <iostream>

#include "TransformComponent.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Graphics/Frustum.h>
#include <Engine/SceneGraph/Entity.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/GraphicsHelper/RenderState.h>
#include <Engine/GraphicsHelper/ShadowRenderer.h>
#include <Engine/Materials/NullMaterial.h>


//=================
// Model Component
//=================


//---------------------------------
// ModelComponent::c-tor
//
// default constructor sets the asset ID
//
ModelComponent::ModelComponent(T_Hash const assetId)
	: m_AssetId(assetId)
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
// ModelComponent::Initialize
//
// Loads the mesh data
//
void ModelComponent::Initialize()
{
	m_Mesh = ResourceManager::GetInstance()->GetAssetData<MeshData>(m_AssetId);
	UpdateMaterial();
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

//---------------------------------
// ModelComponent::Draw
//
// If the material is rendered deferred we do our draw call here
//
void ModelComponent::Draw()
{
	if (m_Material == nullptr)
	{
		LOG("ModelComponent::Draw > material is null!", LogLevel::Warning);
		return;
	}

	if (!(m_Material->IsForwardRendered()))
	{
		DrawCall();
	}
}

//---------------------------------
// ModelComponent::DrawForward
//
// If the material is rendered forward we do our draw call here
//
void ModelComponent::DrawForward()
{
	if (m_Material == nullptr)
	{
		LOG("ModelComponent::Draw > material is null", LogLevel::Warning);
		return;
	}

	if (m_Material->IsForwardRendered())
	{
		DrawCall();
	}
}

//---------------------------------
// ModelComponent::DrawShadow
//
// Render our mesh with a null material from the perspective of a light source
//
void ModelComponent::DrawShadow()
{
	// #todo: implement culling

	NullMaterial* const nullMat = ShadowRenderer::GetInstance()->GetNullMaterial();
	mat4 matWVP = ShadowRenderer::GetInstance()->GetLightVP();

	MeshSurface const* surface = m_Mesh->GetSurface(nullMat);
	STATE->BindVertexArray(surface->GetVertexArray());

	nullMat->UploadVariables(m_pEntity->GetTransform()->GetWorld(), matWVP);

	STATE->DrawElements(GL_TRIANGLES, static_cast<uint32>(m_Mesh->GetIndexCount()), DataTypeInfo::GetTypeId(m_Mesh->GetIndexDataType()), 0);
}

//---------------------------------
// ModelComponent::DrawCall
//
// Performs the draw call, assuming we can't cull our mesh
//
void ModelComponent::DrawCall()
{
	//Frustum culling
	switch (m_CullMode)
	{
	case CullMode::SPHERE:
	{
		Sphere const& filterSphere = m_Mesh->GetBoundingSphere();
		//auto filterSphere = m_pMeshFilter->GetBoundingSphere();
		vec3 scale = TRANSFORM->GetScale();
		float maxScale = std::max(scale.x, std::max(scale.y, scale.z));
		Sphere objSphere = Sphere(GetTransform()->GetPosition() + filterSphere.pos, filterSphere.radius*maxScale);
		if (CAMERA->GetFrustum()->ContainsSphere(objSphere) == VolumeCheck::OUTSIDE)
		{
			return;
		}
	}
	case CullMode::DISABLED:
		break;
	default:
		break;
	}

	//Get Vertex Object
	MeshSurface const* surface = m_Mesh->GetSurface(m_Material);
	STATE->BindVertexArray(surface->GetVertexArray());
	
	m_Material->UploadVariables(m_pEntity->GetTransform()->GetWorld());

	// Draw 
	STATE->SetDepthEnabled(true);
	STATE->DrawElements(GL_TRIANGLES, static_cast<uint32>(m_Mesh->GetIndexCount()), DataTypeInfo::GetTypeId(m_Mesh->GetIndexDataType()), 0);
}