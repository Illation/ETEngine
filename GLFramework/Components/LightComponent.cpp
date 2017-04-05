#include "stdafx.hpp"
#include "LightComponent.hpp"

#include "../SceneGraph/Entity.hpp"
#include "../GraphicsHelper/LightVolume.hpp"

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
void LightComponent::UploadVariables(GLuint shaderProgram, unsigned index)
{
	if (m_PositionUpdated || m_Light->m_Update)
	{
		m_Light->UploadVariables(shaderProgram, GetTransform(), index);
		m_Light->m_Update = false;

		m_PositionUpdated = false;
	}
}

void PointLight::UploadVariables(GLuint program, TransformComponent* comp, unsigned index)
{
	string idxStr = to_string(index);
	string ligStr = "pointLights[";

	vec3 pos = comp->GetPosition();
	vec3 col = color*brightness;

	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Position").c_str()), pos.x, pos.y, pos.z);
	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Color").c_str()), col.x, col.y, col.z);
	glUniform1f(glGetUniformLocation(program, 
		(ligStr + idxStr + "].Radius").c_str()), radius);
}
void PointLight::DrawVolume(TransformComponent* pTransform)
{
	LightVolume::GetInstance()->Draw(pTransform->GetPosition(), radius);
}

void DirectionalLight::UploadVariables(GLuint program, TransformComponent* comp, unsigned index)
{
	string idxStr = to_string(index);
	string ligStr = "dirLights[";

	vec3 direction = comp->GetForward();
	vec3 col = color*brightness;

	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Direction").c_str()), direction.x, direction.y, direction.z);
	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Color").c_str()), col.x, col.y, col.z);
}