#include "stdafx.h"
#include "PlanetTestScene.h"

#include <random>

#include <Demo/Planets/Moon.h>
#include <Demo/Planets/Earth.h>
#include <Demo/Materials/EmissiveMaterial.h>

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/FrameBuffer.h>
#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Graphics/Light.h>
#include <Engine/GraphicsHelper/TextRenderer.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Components/ModelComponent.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/Prefabs/OrbitCamera.h>
#include <Engine/SceneGraph/Entity.h>
#include <Engine/PlanetTech/StarField.h>
#include <Engine/Helper/ScreenshotCapture.h>


PlanetTestScene::PlanetTestScene() : AbstractScene("PlanetTestScene")
{
}
PlanetTestScene::~PlanetTestScene()
{
}

void PlanetTestScene::Initialize()
{
	//Fonts
	//**************************
	m_pDebugFont = ResourceManager::GetInstance()->GetAssetData<SpriteFont>("Consolas_32.fnt"_hash);

	//Camera
	//**************************

	//Materials
	//**************************

	//Skybox
	//**************************
	SetSkybox("Milkyway_small.hdr"_hash);

	StarField* pStarField = new StarField("HYGmxyz.json"_hash);
	AddEntity(pStarField);

	//Models
	//*************************
	m_pPlanet = new Moon();
	AddEntity(m_pPlanet);//Planet is initialized
	CAMERA->GetTransform()->SetPosition(0, 0, -(m_pPlanet->GetRadius() + 10));

	//Lights
	//**************************
	m_pLigEntity = new Entity();
	m_pLight = new DirectionalLight(vec3(1, 1, 1), 283.f);
	//m_pLight->SetShadowEnabled(true);
	auto pLightComp = new LightComponent(m_pLight);
	m_pPlanet->SetSunlight(pLightComp);//Associate this light with the sun for the atmosphere
	m_pLigEntity->AddComponent(pLightComp);
	m_pLigEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	//m_pLigEntity->GetTransform()->SetRotation(etm::lookAt())
	AddEntity(m_pLigEntity);

	m_PostProcessingSettings.exposure = 0.01f;
}

void PlanetTestScene::Update()
{
	//LOG("FPS: " + to_string(TIME->FPS()));

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_2)) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), TIME->DeltaTime()*0.1f));
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_8)) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), -TIME->DeltaTime()*0.1f));
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_4)) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), TIME->DeltaTime()*0.1f));
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_KP_6)) == E_KeyState::Down)
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), -TIME->DeltaTime()*0.1f));
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_j)) == E_KeyState::Down)
	{
		CAMERA->SetFieldOfView(CAMERA->GetFOV() + TIME->DeltaTime()*10);
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_k)) == E_KeyState::Down)
	{
		CAMERA->SetFieldOfView(CAMERA->GetFOV() - TIME->DeltaTime()*10);
	}

	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_0)) == E_KeyState::Pressed)
	{
		ScreenshotCapture::GetInstance()->Take();
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

	//Calculate far plane based on planet
	float radius = std::max(m_pPlanet->GetRadius() + m_pPlanet->GetMaxHeight(), m_pPlanet->GetRadius() + m_pPlanet->GetAtmosphereHeight());
	float altitude = etm::distance(m_pPlanet->GetTransform()->GetPosition(), CAMERA->GetTransform()->GetPosition()) - m_pPlanet->GetRadius();
	CAMERA->SetFarClippingPlane((sqrtf(powf(m_pPlanet->GetRadius() + altitude, 2) - powf(m_pPlanet->GetRadius(), 2)) +
		sqrtf(powf(radius, 2) - powf(m_pPlanet->GetRadius(), 2)))*10);
	CAMERA->SetNearClippingPlane(CAMERA->GetFarPlane()*0.000003f);

	SETTINGS->Window.VSync(true);
}

void PlanetTestScene::Draw()
{
	//TextRenderer::GetInstance()->SetFont(m_pDebugFont.get());
	//TextRenderer::GetInstance()->SetColor(vec4(1, 0.3f, 0.3f, 1));
	//std::string textOutput = "FPS: " + std::to_string( PERFORMANCE->GetRegularFPS() );
	//TextRenderer::GetInstance()->DrawText( textOutput, vec2(20, 20));
	//TextRenderer::GetInstance()->SetColor(vec4(1, 1, 1, 1));
	//textOutput = "Frame ms: " + std::to_string( PERFORMANCE->GetFrameMS() );
	//TextRenderer::GetInstance()->DrawText( textOutput, vec2(20, 50));
	//textOutput = "Draw calls: " + std::to_string( PERFORMANCE->m_PrevDrawCalls );
	//TextRenderer::GetInstance()->DrawText(textOutput, vec2(20, 80));

	//float altitude = etm::distance(m_pPlanet->GetTransform()->GetPosition(), CAMERA->GetTransform()->GetPosition()) - m_pPlanet->GetRadius();
	//textOutput = "Altitude: " + std::to_string( altitude );
	//TextRenderer::GetInstance()->DrawText( textOutput, vec2(20, 110));
	//textOutput = "Vertices: " + std::to_string( m_pPlanet->GetVertexCount() );
	//TextRenderer::GetInstance()->DrawText( textOutput, vec2(20, 140));
}

void PlanetTestScene::DrawForward()
{
}

void PlanetTestScene::PostDraw()
{
}