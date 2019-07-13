#include "stdafx.h"

#include "Planet.h"

#include "Patch.h"
#include "Atmosphere.h"
#include "Triangulator.h"

#include <limits>

#include <Engine/Content/TextureLoader.hpp>

#include <Engine/Graphics/ShaderData.hpp>
#include <Engine/Graphics/TextureData.hpp>
#include <Engine/Graphics/Frustum.hpp>

#include <Engine/GraphicsHelper/RenderPipeline.hpp>
#include <Engine/GraphicsHelper/RenderState.hpp>

#include <Engine/Components/TransformComponent.hpp>
#include <Engine/Components/CameraComponent.hpp>


Planet::Planet()
{
	m_pTriangulator = new Triangulator(this);
	m_pPatch = new Patch(4);
	m_pPatch->SetPlanet(this);
}
Planet::~Planet()
{
	SafeDelete(m_pPatch);
	SafeDelete(m_pTriangulator);
}

void Planet::Initialize()
{
	GetTransform()->SetRotation(GetTransform()->GetRotation() * quat(vec3(0.0f, 1.0f, 0.0f), etm::radians(270.f)));

	//LoadTextures
	TextureLoader* pTL = ContentManager::GetLoader<TextureLoader, TextureData>();
	pTL->UseSrgb(true);
	LoadPlanet();

	m_pDetail1 = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/MoonDetail1.jpg");
	m_pDetail2 = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/MoonDetail2.jpg");
	m_pHeightDetail = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/MoonHeightDetail1.jpg");
	pTL->UseSrgb(false);

	m_pTriangulator->Init();
	m_pPatch->Init();
}

void Planet::Update()
{
	GetTransform()->SetPosition(0, 0, 0);
	if (INPUT->IsKeyboardKeyPressed('R'))m_Rotate = !m_Rotate;
	if(m_Rotate)
	{
		GetTransform()->SetRotation(GetTransform()->GetRotation() * quat(vec3::UP, -(GLfloat)TIME->DeltaTime() * 0.01f ));
	}

	// #todo should happen after transform update maybe at the beginning of draw

	//Change Planet Geometry
	//**********************
	if (m_pTriangulator->Update())
	{
		//Change the actual vertex positions
		m_pTriangulator->GenerateGeometry();
		//Bind patch instances
		m_pPatch->BindInstances(m_pTriangulator->m_Positions);
		m_pPatch->UploadDistanceLUT(m_pTriangulator->m_DistanceLUT);
	}
}

void Planet::Draw()
{
	STATE->SetCullEnabled(false);
	m_pPatch->Draw();
	STATE->SetCullEnabled(true);
}
void Planet::DrawForward()
{
	if (m_pAtmopshere)
	{
		m_pAtmopshere->Draw(this, m_AtmRadius);
	}
}

int32 Planet::GetVertexCount()
{
	return m_pTriangulator->GetVertexCount()*m_pPatch->GetVertexCount();
}

void Planet::SetSunlight(LightComponent* pLight)
{
	if (m_pAtmopshere)m_pAtmopshere->SetSunlight(pLight);
}
