#include "stdafx.hpp"
#include "LightMaterial.hpp"
#include "../../Graphics/TextureData.hpp"
#include "../../Graphics/ShaderData.hpp"
#include "../../Graphics/MeshFilter.hpp"
#include "../../Graphics/FrameBuffer.hpp"
#include "../../Framebuffers/Gbuffer.hpp"

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
}
void LightMaterial::AccessShaderAttributes()
{
	m_uCol = glGetUniformLocation(m_Shader->GetProgram(), "Color");
	m_uPosition = glGetUniformLocation(m_Shader->GetProgram(), "Position");
	m_uRadius = glGetUniformLocation(m_Shader->GetProgram(), "Radius");

	m_uCamPos = glGetUniformLocation(m_Shader->GetProgram(), "camPos");
}
void LightMaterial::UploadDerivedVariables()
{
	//Assume the Gbuffer textures are bound from Gbuffer drawcall
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texPosAO"), 0);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texNormMetSpec"), 1);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texBaseColRough"), 2);

	auto gbufferTex = SCENE->GetGBuffer()->GetTextures();
	for (size_t i = 0; i < gbufferTex.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gbufferTex[i]->GetHandle());
	}

	glUniform3f(m_uPosition, m_Position.x, m_Position.y, m_Position.z);
	glUniform3f(m_uCol, m_Color.x, m_Color.y, m_Color.z);
	glUniform1f(m_uRadius, m_Radius);

	glm::vec3 cPos = CAMERA->GetTransform()->GetPosition();
	glUniform3f(m_uCamPos, cPos.x, cPos.y, cPos.z);
}