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
	SafeDelete(m_Light);
}

void LightComponent::Initialize()
{
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
void LightComponent::UploadVariables(GLuint shaderProgram, uint32 index)
{
	if (m_PositionUpdated || m_Light->m_Update)
	{
		m_Light->UploadVariables(shaderProgram, GetTransform(), index);
		m_Light->m_Update = false;

		m_PositionUpdated = false;
	}
}