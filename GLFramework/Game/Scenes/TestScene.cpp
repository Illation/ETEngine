#include "stdafx.hpp"

#include "TestScene.hpp"

#include "../../SceneGraph/Entity.hpp"

#include "../Materials/TexPBRMaterial.hpp"
#include "../Materials/EmissiveMaterial.hpp"

#include "../../Graphics\FrameBuffer.hpp"
#include "../../Framebuffers\Gbuffer.hpp"
#include "../../Framebuffers\HDRframeBuffer.hpp"

#include "../../Components/ModelComponent.hpp"
#include "../../Components/LightComponent.hpp"

#include <random>

TestScene::TestScene() : AbstractScene("TestScene")
{
}
TestScene::~TestScene()
{
	SafeDelete(m_pMat);
	SafeDelete(m_pLightMat);
}

void TestScene::Initialize()
{
	//Materials
	//**************************
	m_pMat = new TexPBRMaterial(
		"Resources/Textures/BaseColor.png",
		"Resources/Textures/Roughness.png",
		"Resources/Textures/Metalness.png",
		"Resources/Textures/AOMap.png",
		"Resources/Textures/NormalMap.png");
	m_pMat->SetSpecular(0.5f);

	m_pLightMat = new EmissiveMaterial(glm::vec3(500));

	//SetSkybox("Resources/Textures/skybox/sb.jpg");
	//SetSkybox("Resources/Textures/TropicalRuins_3k.hdr");
	SetSkybox("Resources/Textures/Ice_Lake_Ref.hdr");
	//Models
	//*************************
	unsigned amountPerRow = 20;
	float distance = 3.5f;
	float start = (amountPerRow / 2)*(-distance);
	for (size_t i = 0; i < amountPerRow; i++)
	{
		for (size_t j = 0; j < amountPerRow; j++)
		{
			auto pModelComp = new ModelComponent("Resources/Models/helmet.dae");
			pModelComp->SetMaterial(m_pMat);
			auto pHelmet = new Entity();
			pHelmet->AddComponent(pModelComp);
			pHelmet->GetTransform()->Translate(vec3(start+i*distance, 0, start+j*distance));
			AddEntity(pHelmet);
		}
	}
	//Lights
	//**************************

	//Directional
	auto pModelComp1 = new ModelComponent("Resources/Models/sphere.dae");
	pModelComp1->SetMaterial(m_pLightMat);
	auto pLigEntity = new Entity();
	pLigEntity->AddComponent(pModelComp1);
	pLigEntity->AddComponent(new LightComponent(
		new DirectionalLight(vec3(1, 1, 1), 0.0004f)));
	pLigEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	AddEntity(pLigEntity);

	//Moveable point light
	auto pModelComp2 = new ModelComponent("Resources/Models/sphere.dae");
	pModelComp2->SetMaterial(m_pLightMat);
	m_pLigEnt = new Entity();
	m_pLigEnt->AddComponent(pModelComp2);
	m_pLight = new PointLight(vec3(1, 0.8f, 0.8f), 1.f, 10.f);
	m_pLigEnt->AddComponent(new LightComponent(m_pLight));
	m_pLigEnt->GetTransform()->Translate(vec3(1, -2, -1));
	m_pLigEnt->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	AddEntity(m_pLigEnt);
	
	//Random point light field
	random_device rd;
	auto gen= mt19937(rd());
	auto disXZ = uniform_real_distribution<float>(start, -start);
	auto disY = uniform_real_distribution<float>(-0.33f, 1.f);
	auto disC = uniform_real_distribution<float>(0.1f, 0.2f);
	auto disI = uniform_real_distribution<float>(0.5f, 1.0f);
	for (size_t i = 0; i < 25; i++)
	{
		auto pLigMod = new ModelComponent("Resources/Models/sphere.dae");
		pLigMod->SetMaterial(m_pLightMat);
		auto pLigEnt = new Entity();
		pLigEnt->AddComponent(new LightComponent(new PointLight(
			normalize(vec3(disC(gen), disC(gen), disC(gen))), disI(gen)*500, 3.5f)));
		pLigEnt->AddComponent(pLigMod);
		pLigEnt->GetTransform()->Translate(vec3(disXZ(gen), -disY(gen)*3, disXZ(gen)));
		pLigEnt->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
		AddEntity(pLigEnt);
	}

	SETTINGS->Window.VSync(false);
}

void TestScene::Update()
{
	//LOGGER::Log("FPS: " + to_string(TIME->FPS()));
	cout << "FPS: " + to_string(TIME->FPS()) << endl;;

	//Move light
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_2))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, 0, -1)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_8))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, 0, 1)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_4))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(-1, 0, 0)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_6))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(1, 0, 0)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_MINUS))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, -1, 0)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_PLUS))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, 1, 0)*TIME->DeltaTime());
	}

	//Change light settings
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_1))
	{
		float newRad = m_pLight->GetRadius() * 4;
		m_pLight->SetRadius(m_pLight->GetRadius()-(newRad-m_pLight->GetRadius())*TIME->DeltaTime());
		LOGGER::Log("Linear: " + to_string(m_pLight->GetRadius()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_7))
	{
		float newRad = m_pLight->GetRadius() * 4;
		m_pLight->SetRadius(m_pLight->GetRadius()+(newRad-m_pLight->GetRadius())*TIME->DeltaTime());
		LOGGER::Log("Linear: " + to_string(m_pLight->GetRadius()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_3))
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b-(nB-b)*TIME->DeltaTime());
		LOGGER::Log("Linear: " + to_string(m_pLight->GetBrightness()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_9))
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b+(nB-b)*TIME->DeltaTime());
		LOGGER::Log("Linear: " + to_string(m_pLight->GetBrightness()));
	}
}

void TestScene::Draw()
{
}

void TestScene::DrawForward()
{
}

void TestScene::PostDraw()
{
}