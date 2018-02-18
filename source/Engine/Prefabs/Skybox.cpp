#include "stdafx.hpp"
#include "Skybox.hpp"

#include "../Components/ModelComponent.hpp"
#include "../Materials/SkyboxMaterial.hpp"

Skybox::Skybox(std::string assetFile):m_AssetFile(assetFile)
{
}
Skybox::~Skybox()
{
	SafeDelete(m_pMaterial);
}

void Skybox::Initialize()
{
	//Add them componentz
	m_pMaterial = new SkyboxMaterial(m_AssetFile);
	auto pModel = new ModelComponent("Resources/Models/cube.dae");
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

//CubeMap* Skybox::GetCubeMap()
//{
//	return m_pMaterial->GetCubeMap();
//}
HDRMap* Skybox::GetHDRMap()
{
	return m_pMaterial->GetHDRMap();
}

float Skybox::GetRoughness() { return m_pMaterial->GetRoughness(); }
void Skybox::SetRoughness(float r) { m_pMaterial->SetRoughness(r); }