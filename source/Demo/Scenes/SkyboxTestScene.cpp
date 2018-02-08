#include "stdafx.hpp"

#include "SkyboxTestScene.hpp"

#include "../../Engine/SceneGraph/Entity.hpp"

#include "../Materials/TexPBRMaterial.hpp"
#include "../Materials/EmissiveMaterial.hpp"

#include "../../Engine/Graphics/FrameBuffer.hpp"
#include "../../Engine/Framebuffers/Gbuffer.hpp"
#include "../../Engine/Components/ModelComponent.hpp"
#include "../../Engine/Components/LightComponent.hpp"
#include "../../Engine/Prefabs/Skybox.hpp"
#include "../../Engine/Prefabs/OrbitCamera.hpp"
#include "../../Engine/GraphicsHelper/TextRenderer.hpp"
#include "../../Engine/Graphics/SpriteFont.hpp"

#include <random>
#include "../../Engine/Graphics/Light.hpp"

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

	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_2))
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), TIME->DeltaTime()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_8))
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), -TIME->DeltaTime()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_4))
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), TIME->DeltaTime()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_6))
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), -TIME->DeltaTime()));
	}

	//Change light settings
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_3))
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b - (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + to_string(m_pLight->GetBrightness()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_9))
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b + (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + to_string(m_pLight->GetBrightness()));
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