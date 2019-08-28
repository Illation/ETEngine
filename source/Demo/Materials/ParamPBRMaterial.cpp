#include "stdafx.h"
#include "ParamPBRMaterial.h"

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/MeshFilter.h>


ParamPBRMaterial::ParamPBRMaterial(vec3 baseCol, float roughness, float metal) 
	: Material("Shaders/DefPBRParamShader.glsl")
	, m_BaseColor(baseCol)
	, m_Roughness(roughness)
	, m_Metal(metal)
{}

void ParamPBRMaterial::AccessShaderAttributes()
{
	m_uBaseColor = glGetUniformLocation(m_Shader->GetProgram(), "baseColor");
	m_uRoughness = glGetUniformLocation(m_Shader->GetProgram(), "roughness");
	m_uMetal = glGetUniformLocation(m_Shader->GetProgram(), "metalness");
}

void ParamPBRMaterial::UploadDerivedVariables()
{
	glUniform3f(m_uBaseColor, m_BaseColor.x, m_BaseColor.y, m_BaseColor.z);
	glUniform1f(m_uRoughness, m_Roughness);
	glUniform1f(m_uMetal, m_Metal);
}