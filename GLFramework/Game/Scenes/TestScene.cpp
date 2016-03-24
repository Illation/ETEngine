#include "stdafx.hpp"

#include "TestScene.hpp"

#include "../../SceneGraph/Entity.hpp"

#include "../Materials/GbufferMaterial.hpp"
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
	m_pMat = new GbufferMaterial();
	m_pMat->SetDiffuseTexture("Resources/Textures/BaseColor.png");
	m_pMat->SetSpecularTexture("Resources/Textures/Roughness.png");
	m_pMat->SetNormalTexture("Resources/Textures/NormalMap.png");
	m_pMat->SetDifCol(glm::vec3(1.0f, 1.0f, 1.0f));
	m_pMat->SetSpecCol(glm::vec3(1.0f, 1.0f, 1.0f)*1.0f);

	m_pLightMat = new EmissiveMaterial();

	//Models
	//*************************
	unsigned amountPerRow = 15;
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
		new DirectionalLight(vec3(1, 1, 1)*0.0004f, vec3(0.5, 1, 0.5))));
	pLigEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	AddEntity(pLigEntity);

	//Moveable point light
	auto pModelComp2 = new ModelComponent("Resources/Models/sphere.dae");
	pModelComp2->SetMaterial(m_pLightMat);
	m_pLigEnt = new Entity();
	m_pLigEnt->AddComponent(pModelComp2);
	m_pLight = new PointLight(vec3(1, 0.8f, 0.8f)*200.f, 59.f, 568.f);
	m_pLigEnt->AddComponent(new LightComponent(m_pLight));
	m_pLigEnt->GetTransform()->Translate(vec3(1, -2, -1));
	m_pLigEnt->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	AddEntity(m_pLigEnt);
	
	//Random point light field
	random_device rd;
	auto gen= mt19937(rd());
	auto disXZ = uniform_real_distribution<float>(start, -start);
	auto disY = uniform_real_distribution<float>(1, 3);
	auto disC = uniform_real_distribution<float>(0.2f, 1);
	auto disI = uniform_real_distribution<float>(500, 1000);
	for (size_t i = 0; i < 0; i++)
	{
		auto pLigMod = new ModelComponent("Resources/Models/sphere.dae");
		pLigMod->SetMaterial(m_pLightMat);
		auto pLigEnt = new Entity();
		pLigEnt->AddComponent(new LightComponent(new PointLight(
			normalize(vec3(disC(gen), disC(gen), disC(gen)))*disI(gen), 59.f, 568.f)));
		pLigEnt->AddComponent(pLigMod);
		pLigEnt->GetTransform()->Translate(vec3(disXZ(gen), -disY(gen), disXZ(gen)));
		pLigEnt->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
		AddEntity(pLigEnt);
	}

	SETTINGS->Window.VSync(true);
}

void TestScene::Update()
{
	//LOGGER::Log("FPS: " + to_string(TIME->FPS()));
	//cout << "FPS: " + to_string(TIME->FPS()) << endl;;

	//Move light
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_2))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, 0, -0.1f));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_8))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, 0, 0.1f));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_4))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(-0.1f, 0, 0));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_6))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0.1f, 0, 0));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_MINUS))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, -0.1f, 0));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_PLUS))
	{
		m_pLigEnt->GetTransform()->Translate(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, 0.1f, 0));
	}

	//Change light settings
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_1))
	{
		m_pLight->SetLinear(m_pLight->GetLinear() / 1.07f);
		LOGGER::Log("Linear: " + to_string(m_pLight->GetLinear()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_7))
	{
		m_pLight->SetLinear(m_pLight->GetLinear() * 1.07f);
		LOGGER::Log("Linear: " + to_string(m_pLight->GetLinear()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_3))
	{
		m_pLight->SetQuadratic(m_pLight->GetQuadratic() / 1.07f);
		LOGGER::Log("Quadratic: " + to_string(m_pLight->GetQuadratic()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_9))
	{
		m_pLight->SetQuadratic(m_pLight->GetQuadratic() * 1.07f);
		LOGGER::Log("Quadratic: " + to_string(m_pLight->GetQuadratic()));
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