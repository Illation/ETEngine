#include "stdafx.hpp"
#include "LightMaterial.hpp"
#include "../Graphics/TextureData.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/MeshFilter.hpp"
#include "../Graphics/FrameBuffer.hpp"
#include "../Framebuffers/Gbuffer.hpp"

LightMaterial::LightMaterial(glm::vec3 col):
	Material("Shaders/FwdLightPointShader.glsl"),
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
}
void LightMaterial::AccessShaderAttributes()
{
	m_uCol = glGetUniformLocation(m_Shader->GetProgram(), "Color");
	m_uPosition = glGetUniformLocation(m_Shader->GetProgram(), "Position");
	m_uRadius = glGetUniformLocation(m_Shader->GetProgram(), "Radius");

	m_uCamPos = glGetUniformLocation(m_Shader->GetProgram(), "camPos");
	m_uProjA = glGetUniformLocation(m_Shader->GetProgram(), "projectionA");
	m_uProjB = glGetUniformLocation(m_Shader->GetProgram(), "projectionB");
	m_uViewProjInv = glGetUniformLocation(m_Shader->GetProgram(), "viewProjInv");
}
void LightMaterial::UploadDerivedVariables()
{
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texDepth"), 0);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texPosAO"), 1);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texNormMetSpec"), 2);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texBaseColRough"), 3);
	auto gbufferTex = SCENE->GetGBuffer()->GetTextures();
	for (size_t i = 0; i < gbufferTex.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}
	//for position reconstruction
	glUniform1f(m_uProjA, CAMERA->GetDepthProjA());
	glUniform1f(m_uProjB, CAMERA->GetDepthProjB());
	glUniformMatrix4fv(m_uViewProjInv, 1, GL_FALSE, glm::value_ptr(CAMERA->GetStatViewProjInv()));
	glUniform3fv(m_uCamPos, 1, glm::value_ptr(CAMERA->GetTransform()->GetPosition()));

	glUniform3f(m_uPosition, m_Position.x, m_Position.y, m_Position.z);
	glUniform3f(m_uCol, m_Color.x, m_Color.y, m_Color.z);
	glUniform1f(m_uRadius, m_Radius);
}