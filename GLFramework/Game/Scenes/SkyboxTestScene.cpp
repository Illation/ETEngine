#include "stdafx.hpp"

#include "SkyboxTestScene.hpp"

#include "../../SceneGraph/Entity.hpp"

#include "../Materials/GbufferMaterial.hpp"
#include "../Materials/EmissiveMaterial.hpp"

#include "../../Graphics\FrameBuffer.hpp"
#include "../../Framebuffers\Gbuffer.hpp"
#include "../../Framebuffers\HDRframeBuffer.hpp"

#include "../../Components/ModelComponent.hpp"
#include "../../Components/LightComponent.hpp"

#include "../../Prefabs/Skybox.hpp"

#include <random>

SkyboxTestScene::SkyboxTestScene() : AbstractScene("SkyboxTestScene")
{
}
SkyboxTestScene::~SkyboxTestScene()
{
	SafeDelete(m_pMat);
	SafeDelete(m_pMat2);
}

void SkyboxTestScene::Initialize()
{
	//Materials
	//**************************
	m_pMat = new GbufferMaterial();
	m_pMat->SetDiffuseTexture("Resources/Textures/BaseColor.png");
	m_pMat->SetSpecularTexture("Resources/Textures/Roughness.png");
	m_pMat->SetNormalTexture("Resources/Textures/NormalMap.png");
	m_pMat->SetDifCol(glm::vec3(1.0f, 1.0f, 1.0f));
	m_pMat->SetSpecCol(glm::vec3(1.0f, 1.0f, 1.0f)*1.0f);

	m_pMat2 = new GbufferMaterial();
	m_pMat2->SetDifCol(glm::vec3(1.0f, 1.0f, 1.0f)*0.8f);
	m_pMat2->SetSpecCol(glm::vec3(1.0f, 1.0f, 1.0f)*0.5f);
	//Skybox
	//**************************
	m_pSkybox = new Skybox("Resources/Textures/skybox/sb.jpg");
	AddEntity(m_pSkybox);

	//Models
	//*************************
	unsigned amountPerRow = 2;
	float distance = 3.5f;
	float start = (amountPerRow / 2)*(-distance);
	for (size_t i = 0; i < amountPerRow; i++)
	{
		for (size_t j = 0; j < amountPerRow; j++)
		{
			if (i % 2 == 0)
			{
				auto pModelComp = new ModelComponent("Resources/Models/helmet.dae");
				pModelComp->SetMaterial(m_pMat);
				auto pHelmet = new Entity();
				pHelmet->AddComponent(pModelComp);
				pHelmet->GetTransform()->Translate(vec3(start + i*distance, 0, start + j*distance));
				AddEntity(pHelmet);
			}
			else
			{
				auto pModelComp = new ModelComponent("Resources/Models/monkey.dae");
				pModelComp->SetMaterial(m_pMat2);
				auto pHelmet = new Entity();
				pHelmet->AddComponent(pModelComp);
				pHelmet->GetTransform()->Translate(vec3(start + i*distance, 0, start + j*distance));
				AddEntity(pHelmet);
			}
		}
	}
	//Lights
	//**************************
	
	//Directional
	auto pLigEntity = new Entity();
	pLigEntity->AddComponent(new LightComponent(
		new DirectionalLight(vec3(1, 1, 1)*0.99f, vec3(0.5, 1, 0.5))));
	pLigEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	AddEntity(pLigEntity);


	SETTINGS->Window.VSync(false);
}

void SkyboxTestScene::Update()
{
	//LOGGER::Log("FPS: " + to_string(TIME->FPS()));
	//cout << "FPS: " + to_string(TIME->FPS()) << endl;;
}

void SkyboxTestScene::Draw()
{
}

void SkyboxTestScene::DrawForward()
{
}

void SkyboxTestScene::PostDraw()
{
}