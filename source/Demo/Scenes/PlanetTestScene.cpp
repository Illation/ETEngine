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
#include "../../Engine/PlanetTech/Types/Moon.hpp"
#include "../../Engine/PlanetTech/Types/Earth.hpp"
#include "../../Engine/Graphics/Light.hpp"

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

	//Models
	//*************************
	m_pPlanet = new Moon();
	AddEntity(m_pPlanet);
	CAMERA->GetTransform()->SetPosition(0, 0, -(m_pPlanet->GetRadius() + 10));

	//Lights
	//**************************
	m_pLigEntity = new Entity();
	m_pLight = new DirectionalLight(vec3(1, 1, 1), 0.99f);
	//m_pLight->SetShadowEnabled(true);
	m_pLigEntity->AddComponent(new LightComponent( m_pLight));
	m_pLigEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	//m_pLigEntity->GetTransform()->SetRotation(glm::lookAtLH())
	AddEntity(m_pLigEntity);

	SETTINGS->Window.VSync(true);
}

void PlanetTestScene::Update()
{
	//LOGGER::Log("FPS: " + to_string(TIME->FPS()));
	//cout << "FPS: " + to_string(TIME->FPS()) << endl;

	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_2))
	{
		m_pLigEntity->GetTransform()->RotateEuler(TIME->DeltaTime(), 0, 0);
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_8))
	{
		m_pLigEntity->GetTransform()->RotateEuler(-TIME->DeltaTime(), 0, 0);
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_4))
	{
		m_pLigEntity->GetTransform()->RotateEuler(0, -TIME->DeltaTime(), 0);
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_6))
	{
		m_pLigEntity->GetTransform()->RotateEuler(0, TIME->DeltaTime(), 0);
	}

	//Change light settings
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_3))
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b - (nB - b)*TIME->DeltaTime());
		LOGGER::Log("Linear: " + to_string(m_pLight->GetBrightness()));
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_KP_9))
	{
		float b = m_pLight->GetBrightness();
		float nB = b * 4;
		m_pLight->SetBrightness(b + (nB - b)*TIME->DeltaTime());
		LOGGER::Log("Linear: " + to_string(m_pLight->GetBrightness()));
	}

	//Calculate far plane based on planet
	float radius = max(m_pPlanet->GetRadius() + m_pPlanet->GetMaxHeight(), m_pPlanet->GetRadius() + m_pPlanet->GetAtmosphereHeight());
	float altitude = glm::distance(m_pPlanet->GetTransform()->GetPosition(), CAMERA->GetTransform()->GetPosition()) - m_pPlanet->GetRadius();
	CAMERA->SetFarClippingPlane((sqrtf(powf(m_pPlanet->GetRadius() + altitude, 2) - powf(m_pPlanet->GetRadius(), 2)) +
		sqrtf(powf(radius, 2) - powf(m_pPlanet->GetRadius(), 2)))*10);
	CAMERA->SetNearClippingPlane(CAMERA->GetFarPlane()*0.000003f);
}

void PlanetTestScene::Draw()
{
	TextRenderer::GetInstance()->SetFont(m_pDebugFont);
	TextRenderer::GetInstance()->SetColor(glm::vec4(1, 0.3f, 0.3f, 1));
	TextRenderer::GetInstance()->DrawText("FPS: " + std::to_string(PERFORMANCE->GetRegularFPS()), glm::vec2(20, 20));
	TextRenderer::GetInstance()->SetColor(glm::vec4(1, 1, 1, 1));
	TextRenderer::GetInstance()->DrawText("Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS()), glm::vec2(20, 50));
	TextRenderer::GetInstance()->DrawText("Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls), glm::vec2(20, 80));

	float altitude = glm::distance(m_pPlanet->GetTransform()->GetPosition(), CAMERA->GetTransform()->GetPosition()) - m_pPlanet->GetRadius();
	TextRenderer::GetInstance()->DrawText("Altitude: " + std::to_string(altitude), glm::vec2(20, 110));
	TextRenderer::GetInstance()->DrawText("Vertices: " + std::to_string(m_pPlanet->GetVertexCount()), glm::vec2(20, 140));
}

void PlanetTestScene::DrawForward()
{
}

void PlanetTestScene::PostDraw()
{
}