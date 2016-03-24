#include "stdafx.hpp"
#include "LightComponent.hpp"

#include "../SceneGraph/Entity.hpp"

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

void LightComponent::UploadVariables(GLuint shaderProgram, unsigned index)
{
	if (m_PositionUpdated || m_Light->m_Update)
	{
		vec3 pos = GetTransform()->GetPosition();
		m_Light->UploadVariables(shaderProgram, pos, index);
		m_Light->m_Update = false;

		m_PositionUpdated = false;
	}
}

void PointLight::UploadVariables(GLuint program, vec3 pos, unsigned index)
{
	string idxStr = to_string(index);
	string ligStr = "pointLights[";

	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Position").c_str()), pos.x, pos.y, pos.z);
	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Color").c_str()), color.x, color.y, color.z);
	glUniform1f(glGetUniformLocation(program, 
		(ligStr + idxStr + "].Linear").c_str()), linear);
	glUniform1f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Quadratic").c_str()), quadratic);
}
void DirectionalLight::UploadVariables(GLuint program, vec3 pos, unsigned index)
{
	string idxStr = to_string(index);
	string ligStr = "dirLights[";

	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Direction").c_str()), direction.x, direction.y, direction.z);
	glUniform3f(glGetUniformLocation(program,
		(ligStr + idxStr + "].Color").c_str()), color.x, color.y, color.z);
}