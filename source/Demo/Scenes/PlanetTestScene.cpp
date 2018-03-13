#include "stdafx.hpp"

#include "PlanetTestScene.hpp"

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
#include "../Planets/Moon.hpp"
#include "../Planets/Earth.hpp"
#include "../../Engine/Graphics/Light.hpp"
#include "../Engine/PlanetTech/StarField.h"
#include "../Engine/Helper/ScreenshotCapture.h"

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
	m_pDebugFont = ContentManager::Load<SpriteFont>("Resources/Fonts/Consolas_32.fnt");

	//Camera
	//**************************

	//Materials
	//**************************

	//Skybox
	//**************************
	SetSkybox("Resources/Textures/Milkyway_small.hdr");

	StarField* pStarField = new StarField("Resources/HYGmxyz.json");
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

	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_2))
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), TIME->DeltaTime()*0.1f));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_8))
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 0), -TIME->DeltaTime()*0.1f));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_4))
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), TIME->DeltaTime()*0.1f));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_6))
	{
		m_pLigEntity->GetTransform()->Rotate(quat(vec3(0, 1, 0), -TIME->DeltaTime()*0.1f));
	}
	if (INPUT->IsKeyboardKeyDown('J'))
	{
		CAMERA->SetFieldOfView(CAMERA->GetFOV() + TIME->DeltaTime()*10);
	}
	if (INPUT->IsKeyboardKeyDown('K'))
	{
		CAMERA->SetFieldOfView(CAMERA->GetFOV() - TIME->DeltaTime()*10);
	}

	if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_0))
	{
		ScreenshotCapture::GetInstance()->Take();
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
	//TextRenderer::GetInstance()->SetFont(m_pDebugFont);
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