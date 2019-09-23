#include "stdafx.h"
#include "Planet.h"

#include <limits>

#include "Patch.h"
#include "Atmosphere.h"
#include "Triangulator.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/Frustum.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/Components/TransformComponent.h>
#include <Engine/Components/CameraComponent.h>


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
	LoadPlanet();

	m_pDetail1 = ResourceManager::Instance()->GetAssetData<TextureData>("MoonDetail1.jpg"_hash);
	m_pDetail2 = ResourceManager::Instance()->GetAssetData<TextureData>("MoonDetail2.jpg"_hash);
	m_pHeightDetail = ResourceManager::Instance()->GetAssetData<TextureData>("MoonHeightDetail1.jpg"_hash);

	m_pTriangulator->Init();
	m_pPatch->Init();
}

void Planet::Update()
{
	GetTransform()->SetPosition(0, 0, 0);

	if (INPUT->GetKeyState(E_KbdKey::R) == E_KeyState::Pressed)
	{
		m_Rotate = !m_Rotate;
	}

	if(m_Rotate)
	{
		GetTransform()->SetRotation(GetTransform()->GetRotation() * quat(vec3::UP, -(TIME->DeltaTime() * 0.01f)));
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
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetCullEnabled(false);
	m_pPatch->Draw();
	api->SetCullEnabled(true);
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
