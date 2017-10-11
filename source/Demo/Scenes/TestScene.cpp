#include "stdafx.hpp"

#include "TestScene.hpp"

#include <random>
#include <math.h>

#include "../../Engine/SceneGraph/Entity.hpp"

#include "../Materials/TexPBRMaterial.hpp"
#include "../Materials/EmissiveMaterial.hpp"

#include "../../Engine/Graphics/FrameBuffer.hpp"
#include "../../Engine/Framebuffers/Gbuffer.hpp"

#include "../../Engine/Components/ModelComponent.hpp"
#include "../../Engine/Components/LightComponent.hpp"

#include "../../Engine/GraphicsHelper/TextRenderer.hpp"
#include "../../Engine/Graphics/SpriteFont.hpp"
#include "../../Engine/Graphics/Light.hpp"

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
	//Fonts
	//**************************
	m_pDebugFont = ContentManager::Load<SpriteFont>("Resources/Fonts/Consolas_32.fnt");

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
	uint32 amountPerRow = 20;
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
			pHelmet->GetTransform()->SetPosition(vec3(start+i*distance, 0, start+j*distance));
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
	m_pLigEnt->GetTransform()->SetPosition(vec3(1, -2, -1));
	m_pLigEnt->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	AddEntity(m_pLigEnt);
	
	//Random point light field
	random_device rd;
	auto gen= mt19937(rd());
	auto disXZ = uniform_real_distribution<float>(start, -start);
	auto disY = uniform_real_distribution<float>(-0.33f, 1.f);
	auto disC = uniform_real_distribution<float>(0.1f, 0.2f);
	auto disI = uniform_real_distribution<float>(0.5f, 1.0f);
	auto disA = uniform_real_distribution<float>(0.f, 6.28318530718f);
	auto disR = uniform_real_distribution<float>(0.2f, 1);
	auto disT = uniform_real_distribution<float>(-1, 1);
	for (size_t i = 0; i < 2500; i++)
	{
		auto pLigMod = new ModelComponent("Resources/Models/sphere.dae");
		pLigMod->SetMaterial(m_pLightMat);
		auto pLigEnt = new Entity();

		auto swirl = SwirlyLight();
		swirl.origin = vec3(disXZ(gen), -disY(gen)*3, disXZ(gen));

		auto pPoint = new PointLight(normalize(vec3(disC(gen), disC(gen), disC(gen))), disI(gen) * 500, 3.5f);
		pLigEnt->AddComponent(new LightComponent(pPoint));
		pLigEnt->AddComponent(pLigMod);
		pLigEnt->GetTransform()->SetPosition(swirl.origin);
		pLigEnt->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
		AddEntity(pLigEnt);

		swirl.light = pLigEnt;
		swirl.comp = pPoint;
		swirl.angle = disA(gen);
		swirl.radius = disR(gen);
		swirl.angle2 = disA(gen);
		swirl.radius2 = disR(gen);

		swirl.timeMult1 = disT(gen);
		swirl.timeMult1 += swirl.timeMult1 > 0 ? 0.5f : -0.5f;
		swirl.time1 = disA(gen);

		swirl.timeMult2 = disT(gen);
		swirl.timeMult2 += swirl.timeMult2 > 0 ? 0.5f : -0.5f;
		swirl.time2 = disA(gen);

		m_Lights.push_back(swirl);
	}

	SETTINGS->Window.VSync(true);
}

void TestScene::Update()
{
	for (size_t i = 0; i < m_Lights.size(); ++i)
	{
		m_Lights[i].time1 += TIME->DeltaTime();
		float timeMult1 = m_Lights[i].timeMult1 * (1 + 0.9f*std::cosf(m_Lights[i].time1));
		m_Lights[i].angle += TIME->DeltaTime()*timeMult1;
		m_Lights[i].time2 += TIME->DeltaTime();
		float timeMult2 = m_Lights[i].timeMult2 * (1 + 0.9f*std::cosf(m_Lights[i].time2));
		m_Lights[i].angle2 += TIME->DeltaTime()*0.5f*timeMult2;
		glm::vec3 pos = m_Lights[i].origin + glm::vec3(m_Lights[i].radius*std::cosf(m_Lights[i].angle)
			, -m_Lights[i].radius2*std::sinf(m_Lights[i].angle2), -m_Lights[i].radius*std::sinf(m_Lights[i].angle));
		m_Lights[i].light->GetTransform()->SetPosition(pos);
		m_Lights[i].comp->SetBrightness(m_Lights[i].comp->GetBrightness()+std::cosf(m_Lights[i].angle) * 3.5f);
	}

	//Move light
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_2))
	{
		m_pLigEnt->GetTransform()->SetPosition(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, 0, -1)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_8))
	{
		m_pLigEnt->GetTransform()->SetPosition(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, 0, 1)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_4))
	{
		m_pLigEnt->GetTransform()->SetPosition(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(-1, 0, 0)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_6))
	{
		m_pLigEnt->GetTransform()->SetPosition(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(1, 0, 0)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_MINUS))
	{
		m_pLigEnt->GetTransform()->SetPosition(
			m_pLigEnt->GetTransform()->GetPosition() + vec3(0, -1, 0)*TIME->DeltaTime());
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_PLUS))
	{
		m_pLigEnt->GetTransform()->SetPosition(
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
	TextRenderer::GetInstance()->SetFont(m_pDebugFont);
	TextRenderer::GetInstance()->SetColor(glm::vec4(1, 0.3f, 0.3f, 1));
	TextRenderer::GetInstance()->DrawText("FPS: " + std::to_string(PERFORMANCE->GetRegularFPS()), glm::vec2(20, 20));
	TextRenderer::GetInstance()->SetColor(glm::vec4(1, 1, 1, 1));
	TextRenderer::GetInstance()->DrawText("Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS()), glm::vec2(20, 50));
	TextRenderer::GetInstance()->DrawText("Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls), glm::vec2(20, 80));
	TextRenderer::GetInstance()->DrawText("Lights: " + std::to_string((int32)m_Lights.size()), glm::vec2(20, 110));
}

void TestScene::DrawForward()
{
}

void TestScene::PostDraw()
{
}