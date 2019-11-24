#include "stdafx.h"
#include "EditorScene.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/SpriteFont.h>
#include <EtRendering/GraphicsTypes/FrameBuffer.h>
#include <EtRendering/SceneRendering/TextRenderer.h>
#include <EtRendering/SceneRendering/Gbuffer.h>
#include <EtRendering/Materials/UberMaterial.h>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/Components/ModelComponent.h>
#include <EtFramework/Components/LightComponent.h>
#include <EtFramework/Components/SpriteComponent.h>


EditorScene::~EditorScene()
{
	SafeDelete(m_Mat);
	SafeDelete(m_FloorMat);
}

void EditorScene::Init()
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
	m_Mat->SetOcclusionTexture("kabuto_ao.png"_hash);

	m_FloorMat = new UberMaterial();
	m_FloorMat->SetBaseColor(0.5f);
	m_FloorMat->SetMetallic(0.f);

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
	Entity* lightEntity = new Entity();
	m_Light = new LightComponent(LightComponent::Type::Directional, vec3(1, 1, 1), 2.99f, true);
	lightEntity->AddComponent(m_Light);
	vec3 axis;
	float angle = etm::angleSafeAxis(vec3(1, -3, -1), vec3(1, 0, 1), axis);
	lightEntity->GetTransform()->SetRotation(quat(axis, angle));
	AddEntity(lightEntity);

	lightEntity = new Entity();
	lightEntity->AddComponent(new LightComponent(LightComponent::Type::Directional, vec3(1, 1, 1), 1.5f, false));
	lightEntity->GetTransform()->SetRotation(quat(axis, angle));
	lightEntity->GetTransform()->RotateEuler(0, 1, 0);
	AddEntity(lightEntity);

	CAMERA->GetTransform()->SetPosition(0, 0, -10);
}

void EditorScene::Update()
{
	//LOG("FPS: " + to_string(TIME->FPS()));

	if (INPUT->GetKeyState(E_KbdKey::KP_2) == E_KeyState::Down)
	{
		m_Light->GetTransform()->Rotate(quat(vec3(1, 0, 0), TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_8) == E_KeyState::Down)
	{
		m_Light->GetTransform()->Rotate(quat(vec3(1, 0, 0), -TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_4) == E_KeyState::Down)
	{
		m_Light->GetTransform()->Rotate(quat(vec3(0, 1, 0), TIME->DeltaTime()));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_6) == E_KeyState::Down)
	{
		m_Light->GetTransform()->Rotate(quat(vec3(0, 1, 0), -TIME->DeltaTime()));
	}

	//Change light settings
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
}