#include "stdafx.hpp"

#include "ShadingTestScene.hpp"

#include <random>

#include "../../Engine/SceneGraph/Entity.hpp"
#include "../../Engine/Graphics/SpriteFont.hpp"
#include "../../Engine/GraphicsHelper/TextRenderer.hpp"
#include "../../Engine/Graphics/FrameBuffer.hpp"
#include "../../Engine/Framebuffers/Gbuffer.hpp"
#include "../../Engine/Components/ModelComponent.hpp"
#include "../../Engine/Components/LightComponent.hpp"
#include "../../Engine/Prefabs/Skybox.hpp"

#include "../Materials/TexPBRMaterial.hpp"
#include "../Materials/EmissiveMaterial.hpp"
#include "../Materials/ParamPBRMaterial.hpp"
#include "../../Engine/Graphics/Light.hpp"
#include "../Engine/Components/SpriteComponent.hpp"

ShadingTestScene::ShadingTestScene() : AbstractScene("ShadingTestScene")
{
}
ShadingTestScene::~ShadingTestScene()
{
	SafeDelete(m_pMat);
	SafeDelete(m_pStandMat);
	SafeDelete(m_pEnvMat); 
}

void ShadingTestScene::Initialize()
{
	//Fonts
	//***************************
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

	m_pStandMat = new ParamPBRMaterial(vec3(0.95f, 0.95f, 0.5f), 0.2f, 0);
	m_pEnvMat = new ParamPBRMaterial(vec3(0.5f), 0.6f, 0);

	//Skybox
	//**************************
	SetSkybox("Resources/Textures/Ice_Lake_Ref.hdr");

	//Models
	//*************************
	{
		auto pModelComp = new ModelComponent("Resources/Models/HelmetSettled.dae");
		pModelComp->SetMaterial(m_pMat);
		auto pHelmet = new Entity();
		pHelmet->AddComponent(pModelComp);
		pHelmet->GetTransform()->SetPosition(vec3(0, 0, 0));
		AddEntity(pHelmet);
	}
	{
		auto pModelComp = new ModelComponent("Resources/Models/HelmetStand.dae");
		pModelComp->SetMaterial(m_pStandMat);
		auto pHelmet = new Entity();
		pHelmet->AddComponent(pModelComp);
		pHelmet->GetTransform()->SetPosition(vec3(0, 0, 0));
		AddEntity(pHelmet);
	}
	{
		auto pModelComp = new ModelComponent("Resources/Models/Env.dae");
		pModelComp->SetMaterial(m_pEnvMat);
		auto pHelmet = new Entity();
		pHelmet->AddComponent(pModelComp);
		pHelmet->GetTransform()->SetPosition(vec3(0, 0, 0));
		AddEntity(pHelmet);
	}
	{
		auto pModelComp = new ModelComponent("Resources/Assets/Corset.glb");
		pModelComp->SetMaterial(m_pEnvMat);
		auto pCorset = new Entity();
		pCorset->AddComponent(pModelComp);
		pCorset->GetTransform()->SetPosition(vec3(0, 0, 0));
		AddEntity(pCorset);
	}

	//Lights
	//**************************
	m_pLigEntity = new Entity();
	m_pLight = new DirectionalLight(vec3(1, 1, 1), 2.99f);
	m_pLight->SetShadowEnabled(true);
	m_pLigEntity->AddComponent(new LightComponent( m_pLight));
	vec3 axis;
	float angle = etm::angleSafeAxis( vec3( 1, -3, -1 ), vec3( 1, 0, 1 ), axis );
	m_pLigEntity->GetTransform()->SetRotation(quat(axis, angle));
	AddEntity(m_pLigEntity);

	auto pLigEntity = new Entity();
	auto pLight = new DirectionalLight(vec3(1, 1, 1), 4.5f);
	//pLight->SetShadowEnabled(true);
	pLigEntity->AddComponent(new LightComponent(pLight));
	pLigEntity->GetTransform()->SetRotation(quat(axis, angle));
	pLigEntity->GetTransform()->RotateEuler(0, 1, 0);
	AddEntity(pLigEntity);

	//UI
	//************************
	//auto pSpriteEntity = new Entity();
	//auto pSpriteComponent = new SpriteComponent( "Resources/Textures/sample.png", vec2( 0 ), vec4( 1 ) );
	//pSpriteEntity->AddComponent( pSpriteComponent );
	//pSpriteEntity->GetTransform()->SetPosition( vec3( 1250.5f, 50.5f, 1 ) );
	//pSpriteEntity->GetTransform()->SetRotation( quat(vec3(0, 0, 1), etm::PI_DIV4 ) );
	//pSpriteEntity->GetTransform()->Scale( vec3( 1, 0.85f, 1 ) );
	//AddEntity( pSpriteEntity );

	CAMERA->GetTransform()->SetPosition(0, 0, -10);

	SETTINGS->Window.VSync(false);
}

void ShadingTestScene::Update()
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
		LOG("Linear: " + std::to_string(m_pLight->GetBrightness()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_9))
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b + (nB - b)*TIME->DeltaTime());
		LOG("Linear: " + std::to_string(m_pLight->GetBrightness()));
	}

	SETTINGS->Window.VSync(true);
}

void ShadingTestScene::Draw()
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

void ShadingTestScene::DrawForward()
{
}

void ShadingTestScene::PostDraw()
{
}