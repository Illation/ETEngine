#include "stdafx.h"
#include "LightComponent.h"

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/SceneGraph/SceneManager.h>


//=================
// Light Component
//=================


//---------------------------------
// LightComponent::c-tor
//
LightComponent::LightComponent(Type const type, vec3 const& color, float const brightness, bool const castsShadow)
	: m_Type(type)
	, m_Color(color)
	, m_Brightness(brightness)
	, m_CastsShadow(castsShadow)
{
	ET_ASSERT((!castsShadow) || (type == Type::Directional), "Point light shadows are not supported");
}

//---------------------------------
// LightComponent::Init
//
void LightComponent::Init()
{
	vec3 const col = m_Color * m_Brightness;

	m_LightId = SceneManager::GetInstance()->GetRenderScene().AddLight(col, 
		GetTransform()->GetNodeId(), 
		m_Type == Type::Directional,
		m_CastsShadow);
}

//---------------------------------
// LightComponent::Deinit
//
void LightComponent::Deinit()
{
	SceneManager::GetInstance()->GetRenderScene().RemoveLight(m_LightId);
}

//---------------------------------
// LightComponent::Update
//
void LightComponent::Update()
{
	if (m_ColorChanged)
	{
		vec3 const col = m_Color * m_Brightness;
		SceneManager::GetInstance()->GetRenderScene().UpdateLightColor(m_LightId, col);

		m_ColorChanged = false;
	}
}
