#include "stdafx.h"
#include "LightComponent.h"

#include <Engine/SceneGraph/Entity.h>
#include <Engine/Graphics/Light.h>


LightComponent::LightComponent(Light* light):
	m_Light(light)
{
}
LightComponent::~LightComponent()
{
	m_Light->RemoveFromRenderScene();
	SafeDelete(m_Light);
}

void LightComponent::Initialize()
{
	m_Light->AddToRenderScene(GetTransform());
}
void LightComponent::Update()
{
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
