#include "stdafx.hpp"
#include "LightMaterial.hpp"
#include "../../Graphics/ShaderData.hpp"
#include "../../Graphics/MeshFilter.hpp"


LightMaterial::LightMaterial(glm::vec3 col):
	Material("Resources/Shaders/FwdLightShader.glsl"),
	m_Color(col)
{
	m_LayoutFlags = VertexFlags::POSITION;
	m_DrawForward = true;
}
LightMaterial::~LightMaterial()
{
}

void LightMaterial::LoadTextures()
{
	//Nothing to do
}
void LightMaterial::AccessShaderAttributes()
{
	m_uCol = glGetUniformLocation(m_Shader->GetProgram(), "color");
}
void LightMaterial::UploadDerivedVariables()
{
	glUniform3f(m_uCol, m_Color.x, m_Color.y, m_Color.z);
}