#include "stdafx.h"
#include "LightComponent.h"

#include <Engine/SceneGraph/Entity.h>
#include <Engine/SceneGraph/SceneManager.h>


LightComponent::LightComponent(Light* light):
	m_Light(light)
{
}
LightComponent::~LightComponent()
{
	SceneManager::GetInstance()->GetRenderScene().RemoveLight(m_LightId);
	SafeDelete(m_Light);
}

void LightComponent::Initialize()
{
	vec3 const col = m_Light->GetColor() * m_Light->GetBrightness();
	m_LightId = SceneManager::GetInstance()->GetRenderScene().AddLight(col, 
		GetTransform()->GetNodeId(), 
		m_Light->IsDirectional(), 
		m_Light->IsShadowEnabled());
}
void LightComponent::Update()
{
	vec3 const col = m_Light->GetColor() * m_Light->GetBrightness();
	SceneManager::GetInstance()->GetRenderScene().UpdateLightColor(m_LightId, col);
}
void LightComponent::Draw(){}
void LightComponent::DrawForward(){}

void LightComponent::DrawVolume()
{
	m_Light->DrawVolume(GetTransform());
}
void LightComponent::GenerateShadow()
{
	m_Light->GenerateShadow(GetTransform());
}
