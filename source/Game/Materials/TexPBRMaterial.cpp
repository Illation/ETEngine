#include "stdafx.hpp"

#include "TexPBRMaterial.hpp"

#include "../../Graphics/TextureData.hpp"
#include "../../Graphics/ShaderData.hpp"
#include "../../Graphics/MeshFilter.hpp"

#include "../../Content/TextureLoader.hpp"

TexPBRMaterial::TexPBRMaterial(string bcPath, string roughPath,
	string metalPath, string aoPath, string normPath) :
	Material("Shaders/DefPBRMetShader.glsl"),
	m_TexBCPath(bcPath),
	m_TexRoughPath(roughPath),
	m_TexMetalPath(metalPath),
	m_TexAOPath(aoPath),
	m_TexNormPath(normPath)
{
	m_LayoutFlags = VertexFlags::POSITION | VertexFlags::NORMAL | VertexFlags::TANGENT | VertexFlags::TEXCOORD;
}
TexPBRMaterial::~TexPBRMaterial()
{
}

void TexPBRMaterial::LoadTextures()
{
	TextureLoader* pTL = ContentManager::GetLoader<TextureLoader, TextureData>();
	glUseProgram(m_Shader->GetProgram());
	
	pTL->UseSrgb(true);
	m_TexBaseColor = ContentManager::Load<TextureData>(m_TexBCPath);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texBaseColor"), 0);

	m_TexRoughness = ContentManager::Load<TextureData>(m_TexRoughPath);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texRoughness"), 1);

	pTL->UseSrgb(false);

	m_TexMetalness = ContentManager::Load<TextureData>(m_TexMetalPath);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texMetalness"), 2);

	m_TexAO = ContentManager::Load<TextureData>(m_TexAOPath);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texAO"), 3);

	m_TexNorm = ContentManager::Load<TextureData>(m_TexNormPath);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texNormal"), 4);

	m_OutdatedTextureData = false;
}

void TexPBRMaterial::AccessShaderAttributes()
{
	m_uSpecular = glGetUniformLocation(m_Shader->GetProgram(), "specular");
}

void TexPBRMaterial::UploadDerivedVariables()
{
	//Bind active textures
	if (m_OutdatedTextureData)LoadTextures();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TexBaseColor->GetHandle());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TexRoughness->GetHandle());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_TexMetalness->GetHandle());

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_TexAO->GetHandle());

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_TexNorm->GetHandle());

	//Upload uniforms
	glUniform1f(m_uSpecular, m_Specular);
}