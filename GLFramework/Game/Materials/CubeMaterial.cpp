#include "../../stdafx.hpp"

#include "CubeMaterial.hpp"

#include "../../Graphics/TextureData.hpp"
#include "../../Graphics/ShaderData.hpp"
#include "../../Graphics/MeshFilter.hpp"

CubeMaterial::CubeMaterial(std::string texKpath, std::string texPpath)
	:Material("Resources/cuteCube.glsl")
	,m_TexKittenPath(texKpath)
	,m_TexPuppyPath(texPpath)
{
	m_LayoutFlags = VertexFlags::POSITION | VertexFlags::COLOR | VertexFlags::TEXCOORD;
}
CubeMaterial::~CubeMaterial()
{
}

void CubeMaterial::LoadTextures()
{
	//Load Textures
	m_TexKitten = ContentManager::Load<TextureData>("Resources/sample.png");
	m_TexPuppy = ContentManager::Load<TextureData>("Resources/sample2.png");
	//Bind Textures to shader
	glUseProgram(m_Shader->GetProgram());
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texKitten"), 0);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texPuppy"), 1);
}

void CubeMaterial::AccessShaderAttributes()
{
	m_UniTime = glGetUniformLocation(m_Shader->GetProgram(), "time");
	m_UniColor = glGetUniformLocation(m_Shader->GetProgram(), "overrideColor");
}

void CubeMaterial::UploadDerivedVariables()
{
	//Bind active textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TexKitten->GetHandle());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TexKitten->GetHandle());
	//Upload time
	glUniform1f(m_UniTime, TIME->DeltaTime());
}

void CubeMaterial::UpdateReflectionAtt(glm::mat4 model, glm::vec3 col)
{
	glUniformMatrix4fv(m_UniMatModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3f(m_UniColor, col.x, col.y, col.z);
}