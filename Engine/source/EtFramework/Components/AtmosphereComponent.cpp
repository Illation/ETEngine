#include "stdafx.h"
#include "AtmosphereComponent.h"

#include "TransformComponent.h"

#include <EtFramework/SceneGraph/SceneManager.h>


//======================
// Atmosphere Component
//======================


//---------------------------------
// AtmosphereComponent::c-tor
//
AtmosphereComponent::AtmosphereComponent(T_Hash const assetId, float const height, float const groundHeight)
	: m_AssetId(assetId)
	, m_Height(height)
	, m_GroundHeight(groundHeight)
{ }

//----------------------------------
// AtmosphereComponent::SetSunlight
//
void AtmosphereComponent::SetSunlight(core::T_SlotId const lightId)
{
	if (m_RenderId != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().UpdateAtmosphereLight(m_RenderId, lightId);
	}
}

//---------------------------
// AtmosphereComponent::Init
//
void AtmosphereComponent::Init()
{
	render::AtmosphereInstance atmoInst;
	atmoInst.atmosphereId = m_AssetId;
	atmoInst.nodeId = GetTransform()->GetNodeId();
	atmoInst.height = m_Height;
	atmoInst.groundRadius = m_GroundHeight;

	m_RenderId = SceneManager::GetInstance()->GetRenderScene().AddAtmosphere(atmoInst);
}

//-----------------------------
// AtmosphereComponent::Deinit
//
void AtmosphereComponent::Deinit()
{
	if (m_RenderId != core::INVALID_SLOT_ID)
	{
		SceneManager::GetInstance()->GetRenderScene().RemoveAtmosphere(m_RenderId);
	}
}
