#include "stdafx.h"
#include "EmissiveMaterial.h"

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/MeshFilter.h>


EmissiveMaterial::EmissiveMaterial(vec3 col):
	Material("Shaders/FwdEmissiveShader.glsl"),
	m_Color(col)
{
	m_LayoutFlags = E_VertexFlag::POSITION | E_VertexFlag::NORMAL | E_VertexFlag::TANGENT | E_VertexFlag::TEXCOORD;
	m_DrawForward = true;
}
EmissiveMaterial::~EmissiveMaterial()
{
}

void EmissiveMaterial::LoadTextures()
{
	//Nothing to do
}
void EmissiveMaterial::AccessShaderAttributes()
{
	m_uCol = glGetUniformLocation(m_Shader->GetProgram(), "color");
}
void EmissiveMaterial::UploadDerivedVariables()
{
	
	glUniform3f(m_uCol, m_Color.x, m_Color.y, m_Color.z);
}