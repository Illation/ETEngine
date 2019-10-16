#include "stdafx.h"
#include "SkyboxTestScene.h"

#include <random>

#include <Runtime/Materials/TexPBRMaterial.h>
#include <Runtime/Materials/EmissiveMaterial.h>

#include <EtCore/Content/ResourceManager.h>

#include <Engine/SceneGraph/Entity.h>
#include <Engine/Components/ModelComponent.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/Prefabs/OrbitCamera.h>
#include <Engine/SceneRendering/SceneRenderer.h>
#include <Engine/SceneRendering/Gbuffer.h>
#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Graphics/FrameBuffer.h>
#include <Engine/Graphics/Light.h>


SkyboxTestScene::SkyboxTestScene() : AbstractScene("SkyboxTestScene")
{
}
SkyboxTestScene::~SkyboxTestScene()
{
	SafeDelete(m_pMat);
}

void SkyboxTestScene::Initialize()
{
	//Fonts
	//**************************
	m_pDebugFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Consolas_32.fnt"_hash);

	//Camera
	//**************************
	//auto cam = new OrbitCamera();
	//AddEntity(cam);

	//Materials
	//**************************
	m_pMat = new TexPBRMaterial(
		"kabuto_baseColor.png"_hash,
		"kabuto_roughness.png"_hash,
		"kabuto_metal.png"_hash,
		"kabuto_ao.png"_hash,
		"kabuto_normal.png"_hash);
	m_pMat->SetSpecular(0.5f);

	//Skybox
	//**************************
	SetSkybox("Ice_Lake_Ref.hdr"_hash);

	//Models
	//*************************
	auto pModelComp = new ModelComponent("helmet.dae"_hash);
	pModelComp->SetMaterial(m_pMat);
	auto pHelmet = new Entity();
	pHelmet->AddComponent(pModelComp);
	pHelmet->GetTransform()->SetPosition(vec3(0, 0, 0));
	AddEntity(pHelmet);

	//Lights
	//**************************
	m_pLigEntity = new Entity();
	m_pLight = new DirectionalLight(vec3(1, 1, 1), 0.99f);
	//m_pLight->SetShadowEnabled(true);
	m_pLigEntity->AddComponent(new LightComponent( m_pLight));
	m_pLigEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	//m_pLigEntity->GetTransform()->SetRotation(etm::lookAt())
	AddEntity(m_pLigEntity);
}

void SkyboxTestScene::Update()
{
	//LOG("FPS: " + to_string(TIME->FPS()));

	if (INPUT->GetKeyState(E_KbdKey::KP_2) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_8) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), -TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_4) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_6) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), -TIME->DeltaTime()));
	}

	//Change light settings
	if (INPUT->GetKeyState(E_KbdKey::KP_3) == E_KeyState::Down)
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b - (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(m_pLight->GetBrightness()));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_9) == E_KeyState::Down)
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b + (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(m_pLight->GetBrightness()));
	}
}

void SkyboxTestScene::Draw()
{
	TextRenderer& textRenderer = SceneRenderer::GetCurrent()->GetTextRenderer();

	textRenderer.SetFont(m_pDebugFont.get());
	textRenderer.SetColor(vec4(1, 0.3f, 0.3f, 1));
	std::string outString = "FPS: " + std::to_string(PERFORMANCE->GetRegularFPS());
	textRenderer.DrawText(outString, vec2(20, 20));
	textRenderer.SetColor(vec4(1, 1, 1, 1));
	outString = "Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS());
	textRenderer.DrawText(outString, vec2(20, 50));
	outString = "Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls);
	textRenderer.DrawText(outString, vec2(20, 80));
}

void SkyboxTestScene::DrawForward()
{
}

void SkyboxTestScene::PostDraw()
{
}