#include "stdafx.h"
#include "SkyboxTestScene.h"

#include <random>

#include <Runtime/Materials/TexPBRMaterial.h>
#include <Runtime/Materials/EmissiveMaterial.h>

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/SceneRendering/Gbuffer.h>
#include <EtRendering/GraphicsTypes/SpriteFont.h>
#include <EtRendering/GraphicsTypes/FrameBuffer.h>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/Components/ModelComponent.h>
#include <EtFramework/Components/LightComponent.h>
#include <EtFramework/Templates/OrbitCamera.h>


SkyboxTestScene::SkyboxTestScene() : AbstractScene("SkyboxTestScene")
{
}
SkyboxTestScene::~SkyboxTestScene()
{
	SafeDelete(m_pMat);
}

void SkyboxTestScene::Init()
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
	Entity* const lightEntity = new Entity();
	m_Light = new LightComponent(LightComponent::Type::Directional, vec3(1, 1, 1), 0.99f);
	lightEntity->AddComponent(m_Light);
	AddEntity(lightEntity);
}

void SkyboxTestScene::Update()
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
}
