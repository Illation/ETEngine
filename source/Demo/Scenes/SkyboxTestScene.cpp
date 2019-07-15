#include "stdafx.h"
#include "SkyboxTestScene.h"

#include <random>

#include <Demo/Materials/TexPBRMaterial.h>
#include <Demo/Materials/EmissiveMaterial.h>

#include <Engine/SceneGraph/Entity.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Components/ModelComponent.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/Prefabs/OrbitCamera.h>
#include <Engine/GraphicsHelper/TextRenderer.h>
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
	m_pDebugFont = ContentManager::Load<SpriteFont>("Resources/Fonts/Consolas_32.fnt");

	//Camera
	//**************************
	//auto cam = new OrbitCamera();
	//AddEntity(cam);

	//Materials
	//**************************
	m_pMat = new TexPBRMaterial(
		"Resources/Textures/BaseColor.png",
		"Resources/Textures/Roughness.png",
		"Resources/Textures/Metalness.png",
		"Resources/Textures/AOMap.png",
		"Resources/Textures/NormalMap.png");
	m_pMat->SetSpecular(0.5f);

	//Skybox
	//**************************
	//SetSkybox("Resources/Textures/CharlesRiver_Ref.hdr");
	//SetSkybox("Resources/Textures/Harbour_3_Ref.hdr");
	SetSkybox("Resources/Textures/Ice_Lake_Ref.hdr");
	//SetSkybox("Resources/Textures/TropicalRuins_3k.hdr");
	//SetSkybox("Resources/Textures/WinterForest_Ref.hdr");

	//Models
	//*************************
	auto pModelComp = new ModelComponent("Resources/Models/helmet.dae");
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

	SETTINGS->Window.VSync(false);
}

void SkyboxTestScene::Update()
{
	//LOG("FPS: " + to_string(TIME->FPS()));

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_2)) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_8)) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), -TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_4)) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_6)) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), -TIME->DeltaTime()));
	}

	//Change light settings
	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_3)) == E_KeyState::Down)
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b - (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(m_pLight->GetBrightness()));
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_9)) == E_KeyState::Down)
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b + (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(m_pLight->GetBrightness()));
	}
}

void SkyboxTestScene::Draw()
{
	TextRenderer::GetInstance()->SetFont(m_pDebugFont);
	TextRenderer::GetInstance()->SetColor(vec4(1, 0.3f, 0.3f, 1));
	std::string outString = "FPS: " + std::to_string( PERFORMANCE->GetRegularFPS() );
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 20));
	TextRenderer::GetInstance()->SetColor(vec4(1, 1, 1, 1));
	outString = "Frame ms: " + std::to_string( PERFORMANCE->GetFrameMS() );
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 50));
	outString = "Draw Calls: " + std::to_string( PERFORMANCE->m_PrevDrawCalls );
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 80));
}

void SkyboxTestScene::DrawForward()
{
}

void SkyboxTestScene::PostDraw()
{
}