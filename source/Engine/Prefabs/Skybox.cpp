#include "stdafx.h"
#include "Skybox.h"

#include <Engine/Components/ModelComponent.h>
#include <Engine/Materials/SkyboxMaterial.h>


Skybox::Skybox(T_Hash const assetId)
	: m_AssetId(assetId)
{}

Skybox::~Skybox()
{
	SafeDelete(m_pMaterial);
}

void Skybox::Initialize()
{
	//Add them componentz
	m_pMaterial = new SkyboxMaterial(m_AssetId);

	auto pModel = new ModelComponent("cube.dae"_hash);
	pModel->SetCullMode(ModelComponent::CullMode::DISABLED);//Always draw
	pModel->SetMaterial(m_pMaterial);
	AddComponent(pModel);

	GetTransform()->Scale(vec3(5, 5, 5));
}

void Skybox::Update()
{
	//Make sure the model sits ontop of the camera
	GetTransform()->SetPosition(CAMERA->GetTransform()->GetPosition());
}

void Skybox::DrawForward()
{
	//set opengl depth function to less equal and the backside is drawn
	glDepthFunc(GL_LEQUAL);
}

EnvironmentMap const* Skybox::GetHDRMap() const
{
	return m_pMaterial->GetHDRMap();
}

float Skybox::GetRoughness() const
{ 
	return m_pMaterial->GetRoughness(); 
}

void Skybox::SetRoughness(float r) 
{ 
	m_pMaterial->SetRoughness(r); 
}