#include "stdafx.h"
#include "PlanetTemplate.h"

#include <EtFramework/Components/TransformComponent.h>
#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/SceneGraph/SceneManager.h>


//=================
// Planet Template
//=================


//---------------------------------
// PlanetTemplate::d-tor
//
// Remove rendering data from render::Scene
//
PlanetTemplate::~PlanetTemplate()
{
	if (m_PlanetId != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().RemovePlanet(m_PlanetId);

		if (m_AtmoId != core::INVALID_SLOT_ID)
		{
			SceneManager::GetInstance()->GetRenderScene().RemoveAtmosphere(m_AtmoId);
		}
	}
	else
	{
		ET_ASSERT(m_AtmoId == core::INVALID_SLOT_ID);
	}
}

//---------------------------------
// PlanetTemplate::Initialize
//
// Add rendering data to render::Scene
//
void PlanetTemplate::Initialize()
{
	GetTransform()->SetRotation(GetTransform()->GetRotation() * quat(vec3(0.0f, 1.0f, 0.0f), etm::radians(270.f)));

	// add planet to scene
	render::T_NodeId const nodeId = GetTransform()->GetNodeId();
	m_PlanetId = SceneManager::GetInstance()->GetRenderScene().AddPlanet(m_Params, nodeId);

	// add atmosphere to scene
	if (m_AtmosphereAssetId != 0u)
	{
		render::AtmosphereInstance atmoInst;
		atmoInst.atmosphereId = m_AtmosphereAssetId;
		atmoInst.nodeId = nodeId;
		atmoInst.height = m_AtmosphereHeight;
		atmoInst.groundRadius = m_Params.radius;

		m_AtmoId = SceneManager::GetInstance()->GetRenderScene().AddAtmosphere(atmoInst);
	}
}

//---------------------------------
// PlanetTemplate::Update
//
void PlanetTemplate::Update()
{
	GetTransform()->SetPosition(0, 0, 0);

	if (INPUT->GetKeyState(E_KbdKey::R) == E_KeyState::Pressed)
	{
		m_Rotate = !m_Rotate;
	}

	if (m_Rotate)
	{
		GetTransform()->SetRotation(GetTransform()->GetRotation() * quat(vec3::UP, -(TIME->DeltaTime() * 0.01f)));
	}
}

//---------------------------------
// PlanetTemplate::SetSunlight
//
// if there is an atmosphere associated, we set the light source
//
void PlanetTemplate::SetSunlight(core::T_SlotId const lightId)
{
	if (m_AtmoId != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().UpdateAtmosphereLight(m_AtmoId, lightId);
	}
}

