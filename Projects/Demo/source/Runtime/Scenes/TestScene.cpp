#include "stdafx.h"
#include "TestScene.h"

#include <random>
#include <math.h>

#include <Runtime/Materials/TexPBRMaterial.h>
#include <Runtime/Materials/EmissiveMaterial.h>

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/Gbuffer.h>
#include <EtRendering/GraphicsTypes/FrameBuffer.h>
#include <EtRendering/GraphicsTypes/SpriteFont.h>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/Components/ModelComponent.h>
#include <EtFramework/Components/LightComponent.h>


TestScene::TestScene() : AbstractScene("TestScene")
{
}
TestScene::~TestScene()
{
	SafeDelete(m_pMat);
	SafeDelete(m_pLightMat);
}

void TestScene::Init()
{
	//Fonts
	//**************************
	m_pDebugFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Consolas_32.fnt"_hash);

	//Materials
	//**************************
	m_pMat = new TexPBRMaterial(
		"kabuto_baseColor.png"_hash,
		"kabuto_roughness.png"_hash,
		"kabuto_metal.png"_hash,
		"kabuto_ao.png"_hash,
		"kabuto_normal.png"_hash);
	m_pMat->SetSpecular(0.5f);

	m_pLightMat = new EmissiveMaterial(vec3(500));

	SetSkybox("Ice_Lake_Ref.hdr"_hash);

	//Models
	//*************************
	uint32 amountPerRow = 20;
	float distance = 3.5f;
	float start = (amountPerRow / 2)*(-distance);
	for (size_t i = 0; i < amountPerRow; i++)
	{
		for (size_t j = 0; j < amountPerRow; j++)
		{
			auto pModelComp = new ModelComponent("helmet.dae"_hash);
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
	auto pModelComp1 = new ModelComponent("sphere.dae"_hash);
	pModelComp1->SetMaterial(m_pLightMat);
	auto pLigEntity = new Entity();
	pLigEntity->AddComponent(pModelComp1);
	pLigEntity->AddComponent(new LightComponent(LightComponent::Type::Directional, vec3(1, 1, 1), 0.0004f));
	pLigEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	AddEntity(pLigEntity);

	//Moveable point light
	auto pModelComp2 = new ModelComponent("sphere.dae"_hash);
	pModelComp2->SetMaterial(m_pLightMat);
	auto lightEntity = new Entity();
	lightEntity->AddComponent(pModelComp2);
	m_Light = new LightComponent(LightComponent::Type::Point, vec3(1, 0.8f, 0.8f), 1.f);
	lightEntity->AddComponent(m_Light);
	lightEntity->GetTransform()->SetPosition(vec3(1, -2, -1));
	lightEntity->GetTransform()->Scale(vec3(10.f));
	AddEntity(lightEntity);
	
	//Random point light field
	std::random_device rd;
	auto gen= std::mt19937(rd());
	auto disXZ = std::uniform_real_distribution<float>(start, -start);
	auto disY = std::uniform_real_distribution<float>(-0.33f, 1.f);
	auto disC = std::uniform_real_distribution<float>(0.1f, 0.2f);
	auto disI = std::uniform_real_distribution<float>(0.5f, 1.0f);
	auto disA = std::uniform_real_distribution<float>(0.f, 6.28318530718f);
	auto disR = std::uniform_real_distribution<float>(0.2f, 1);
	auto disT = std::uniform_real_distribution<float>(-1, 1);
	for (size_t i = 0; i < 2500; i++)
	{
		auto pLigMod = new ModelComponent("sphere.dae"_hash);
		pLigMod->SetMaterial(m_pLightMat);
		auto pLigEnt = new Entity();

		auto swirl = SwirlyLight();
		swirl.origin = vec3(disXZ(gen), -disY(gen)*3, disXZ(gen));

		auto pPoint = new LightComponent(LightComponent::Type::Point, normalize(vec3(disC(gen), disC(gen), disC(gen))), disI(gen) * 500);
		pLigEnt->AddComponent(pPoint);
		pLigEnt->AddComponent(pLigMod);
		pLigEnt->GetTransform()->SetPosition(swirl.origin);
		pLigEnt->GetTransform()->Scale(vec3(3.5f));
		AddEntity(pLigEnt);

		swirl.light = pPoint;
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
		vec3 pos = m_Lights[i].origin + vec3(m_Lights[i].radius*std::cosf(m_Lights[i].angle)
			, -m_Lights[i].radius2*std::sinf(m_Lights[i].angle2), -m_Lights[i].radius*std::sinf(m_Lights[i].angle));
		m_Lights[i].light->GetTransform()->SetPosition(pos);
		m_Lights[i].light->SetBrightness(m_Lights[i].light->GetBrightness()+std::cosf(m_Lights[i].angle) * 3.5f);
	}

	//Move light
	if (INPUT->GetKeyState(E_KbdKey::KP_2) == E_KeyState::Down)
	{
		m_Light->GetTransform()->SetPosition( m_Light->GetTransform()->GetPosition() + vec3(0, 0, -1)*TIME->DeltaTime());
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_8) == E_KeyState::Down)
	{
		m_Light->GetTransform()->SetPosition( m_Light->GetTransform()->GetPosition() + vec3(0, 0, 1)*TIME->DeltaTime());
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_4) == E_KeyState::Down)
	{
		m_Light->GetTransform()->SetPosition( m_Light->GetTransform()->GetPosition() + vec3(-1, 0, 0)*TIME->DeltaTime());
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_6) == E_KeyState::Down)
	{
		m_Light->GetTransform()->SetPosition( m_Light->GetTransform()->GetPosition() + vec3(1, 0, 0)*TIME->DeltaTime());
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_Minus) == E_KeyState::Down)
	{
		m_Light->GetTransform()->SetPosition( m_Light->GetTransform()->GetPosition() + vec3(0, -1, 0)*TIME->DeltaTime());
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_Plus) == E_KeyState::Down)
	{
		m_Light->GetTransform()->SetPosition( m_Light->GetTransform()->GetPosition() + vec3(0, 1, 0)*TIME->DeltaTime());
	}

	//Change light settings
	if (INPUT->GetKeyState(E_KbdKey::KP_1) == E_KeyState::Down)
	{
		float const scale = etm::length(m_Light->GetTransform()->GetScale());
		m_Light->GetTransform()->Scale(scale - (scale * 4 - scale)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(etm::length(m_Light->GetTransform()->GetScale())));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_7) == E_KeyState::Down)
	{
		float const scale = etm::length(m_Light->GetTransform()->GetScale());
		m_Light->GetTransform()->Scale(scale + (scale * 4 - scale)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(etm::length(m_Light->GetTransform()->GetScale())));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_3) == E_KeyState::Down)
	{
		float b = m_Light->GetBrightness();
		float nB = b * 4;
		m_Light->SetBrightness(b - (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(m_Light->GetBrightness()));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_9) == E_KeyState::Down)
	{
		float b = m_Light->GetBrightness();
		float nB = b * 4;
		m_Light->SetBrightness(b + (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(m_Light->GetBrightness()));
	}

	TextRenderer& textRenderer = render::ShadedSceneRenderer::GetCurrent()->GetTextRenderer();

	textRenderer.SetFont(m_pDebugFont.get());
	textRenderer.SetColor(vec4(1, 0.3f, 0.3f, 1));
	std::string outString = "FPS: " + std::to_string(PERFORMANCE->GetRegularFPS());
	textRenderer.DrawText(outString, vec2(20, 20));
	textRenderer.SetColor(vec4(1, 1, 1, 1));
	outString = "Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS());
	textRenderer.DrawText(outString, vec2(20, 50));
	outString = "Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls);
	textRenderer.DrawText(outString, vec2(20, 80));
	outString = "Lights: " + std::to_string((int32)m_Lights.size());
	textRenderer.DrawText(outString, vec2(20, 110));
}