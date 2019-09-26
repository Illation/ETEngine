#include "stdafx.h"
#include "EditorScene.h"

#include <EtCore/Content/ResourceManager.h>

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
#include <Engine/Materials/UberMaterial.h>

EditorScene::~EditorScene()
{
	SafeDelete(m_Mat);
}

void EditorScene::Initialize()
{
	//Fonts
	//***************************
	m_pDebugFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Consolas_32.fnt"_hash);

	//Camera
	//**************************
	//auto cam = new OrbitCamera();
	//AddEntity(cam);

	//Materials
	//**************************
	m_Mat = new UberMaterial();
	m_Mat->SetBaseColorTexture("kabuto_baseColor.png"_hash);
	m_Mat->SetNormalTexture("kabuto_normal.png"_hash);
	m_Mat->SetMetallicRoughnessTexture("kabuto_metallic_roughness.png"_hash);
	m_Mat->SetRoughness(1.0f);
	m_Mat->SetMetallic(1.0f);

	m_FloorMat = new UberMaterial();
	m_FloorMat->SetRoughness(1.0f);
	m_FloorMat->SetMetallic(0.0f);

	//Skybox
	//**************************
	SetSkybox("Ice_Lake_Ref.hdr"_hash);

	//Models
	//*************************
	{
		auto pModelComp = new ModelComponent("HelmetSettled.dae"_hash);
		pModelComp->SetMaterial(m_Mat);
		auto pHelmet = new Entity();
		pHelmet->AddComponent(pModelComp);
		pHelmet->GetTransform()->SetPosition(vec3(0, 0, 0));
		AddEntity(pHelmet);
	}
	{
		auto pModelComp = new ModelComponent("HelmetStand.dae"_hash);
		pModelComp->SetMaterial(m_FloorMat);
		auto pHelmet = new Entity();
		pHelmet->AddComponent(pModelComp);
		pHelmet->GetTransform()->SetPosition(vec3(0, 0, 0));
		AddEntity(pHelmet);
	}
	{
		auto pModelComp = new ModelComponent("Env.dae"_hash);
		pModelComp->SetMaterial(m_FloorMat);
		auto pHelmet = new Entity();
		pHelmet->AddComponent(pModelComp);
		pHelmet->GetTransform()->SetPosition(vec3(0, 0, 0));
		AddEntity(pHelmet);
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
	auto pLight = new DirectionalLight(vec3(1, 1, 1), 1.5f);
	//pLight->SetShadowEnabled(true);
	pLigEntity->AddComponent(new LightComponent(pLight));
	pLigEntity->GetTransform()->SetRotation(quat(axis, angle));
	pLigEntity->GetTransform()->RotateEuler(0, 1, 0);
	AddEntity(pLigEntity);

	CAMERA->GetTransform()->SetPosition(0, 0, -10);
}

void EditorScene::Update()
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

void EditorScene::Draw()
{
	TextRenderer::GetInstance()->SetFont(m_pDebugFont.get());
	TextRenderer::GetInstance()->SetColor(vec4(1, 0.3f, 0.3f, 1));
	std::string outString = "FPS: " + std::to_string( PERFORMANCE->GetRegularFPS() );
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 20));
	TextRenderer::GetInstance()->SetColor(vec4(1, 1, 1, 1));
	outString = "Frame ms: " + std::to_string( PERFORMANCE->GetFrameMS() );
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 50));
	outString = "Draw Calls: " + std::to_string( PERFORMANCE->m_PrevDrawCalls );
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 80));
}
