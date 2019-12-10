#include "stdafx.h"
#include "PlanetTestScene.h"

#include <random>

#include <Runtime/Planets/Moon.h>
#include <Runtime/Planets/Earth.h>

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/GraphicsTypes/FrameBuffer.h>
#include <EtRendering/GraphicsTypes/SpriteFont.h>
#include <EtRendering/SceneRendering/TextRenderer.h>

#include <EtFramework/Components/ModelComponent.h>
#include <EtFramework/Components/LightComponent.h>
#include <EtFramework/Templates/OrbitCamera.h>
#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/SceneGraph/SceneManager.h>


void PlanetTestScene::Init()
{
	//Skybox
	//**************************
	SetSkybox("Milkyway_small.hdr"_hash);
	SetStarfield("HYGmxyz.json"_hash);

	//Lights
	//**************************
	auto lightEntity = new Entity();
	m_Light = new LightComponent(LightComponent::Type::Directional, vec3(1, 1, 1), 283.f);
	lightEntity->AddComponent(m_Light);
	lightEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	AddEntity(lightEntity);

	//Models
	//*************************
	m_Planet = new Moon();
	AddEntity(m_Planet);//Planet is initialized
	m_Planet->SetSunlight(m_Light->GetLightId());//Associate this light with the sun for the atmosphere

	CAMERA->GetTransform()->SetPosition(0, 0, -(m_Planet->GetRadius() + 10));

	//Initial exposure
	//*************************
	render::Scene& renderScene = SceneManager::GetInstance()->GetRenderScene();
	PostProcessingSettings ppSettings = renderScene.GetPostProcessingSettings();
	ppSettings.exposure = 0.01f;
	renderScene.SetPostProcessingSettings(ppSettings);
}

void PlanetTestScene::Update()
{
	//LOG("FPS: " + to_string(TIME->FPS()));

	if (INPUT->GetKeyState(E_KbdKey::KP_2) == E_KeyState::Down)
	{
		m_Light->GetTransform()->Rotate(quat(vec3(1, 0, 0), TIME->DeltaTime()*0.1f));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_8) == E_KeyState::Down)
	{
		m_Light->GetTransform()->Rotate(quat(vec3(1, 0, 0), -TIME->DeltaTime()*0.1f));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_4) == E_KeyState::Down)
	{
		m_Light->GetTransform()->Rotate(quat(vec3(0, 1, 0), TIME->DeltaTime()*0.1f));
	}

	if (INPUT->GetKeyState(E_KbdKey::KP_6) == E_KeyState::Down)
	{
		m_Light->GetTransform()->Rotate(quat(vec3(0, 1, 0), -TIME->DeltaTime()*0.1f));
	}

	if (INPUT->GetKeyState(E_KbdKey::J) == E_KeyState::Down)
	{
		CAMERA->SetFieldOfView(CAMERA->GetFieldOfView() + TIME->DeltaTime()*10);
	}

	if (INPUT->GetKeyState(E_KbdKey::K) == E_KeyState::Down)
	{
		CAMERA->SetFieldOfView(CAMERA->GetFieldOfView() - TIME->DeltaTime()*10);
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

	//Calculate far plane based on planet
	float radius = std::max(m_Planet->GetRadius() + m_Planet->GetMaxHeight(), m_Planet->GetRadius() + m_Planet->GetAtmosphereHeight());
	float altitude = etm::distance(m_Planet->GetTransform()->GetPosition(), CAMERA->GetTransform()->GetPosition()) - m_Planet->GetRadius();
	CAMERA->SetFarClippingPlane((sqrtf(powf(m_Planet->GetRadius() + altitude, 2) - powf(m_Planet->GetRadius(), 2)) +
		sqrtf(powf(radius, 2) - powf(m_Planet->GetRadius(), 2)))*10);
	CAMERA->SetNearClippingPlane(CAMERA->GetFarPlane()*0.000003f);
}
