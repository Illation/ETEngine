#include "stdafx.h"
#include "ShadingTestScene.h"

#include <random>

#include <Demo/Materials/TexPBRMaterial.h>
#include <Demo/Materials/EmissiveMaterial.h>
#include <Demo/Materials/ParamPBRMaterial.h>

#include <Engine/SceneGraph/Entity.h>
#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Graphics/FrameBuffer.h>
#include <Engine/Graphics/Light.h>
#include <Engine/GraphicsHelper/TextRenderer.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Components/ModelComponent.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Components/SpriteComponent.h>
#include <Engine/Prefabs/Skybox.h>


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
		"kabuto_baseColor.png"_hash,
		"kabuto_roughness.png"_hash,
		"kabuto_metal.png"_hash,
		"kabuto_ao.png"_hash,
		"kabuto_normal.png"_hash);
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
		//auto pModelComp = new ModelComponent("Resources/Assets/Box.gltf");
		auto pModelComp = new ModelComponent("Resources/Assets/Corset.glb");
		pModelComp->SetMaterial(m_pEnvMat);
		auto pCorset = new Entity();
		pCorset->AddComponent(pModelComp);
		pCorset->GetTransform()->SetPosition(vec3(5, 0, 0));
		pCorset->GetTransform()->Scale(vec3(100, 100, 100));
		pCorset->GetTransform()->Rotate(quat(vec3(1, 0, 0), etm::PI));
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