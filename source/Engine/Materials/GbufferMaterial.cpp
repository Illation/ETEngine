#include "stdafx.hpp"

#include "GbufferMaterial.hpp"

#include "../Graphics/TextureData.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/MeshFilter.hpp"

#include "../Content/TextureLoader.hpp"

GbufferMaterial::GbufferMaterial() :
	Material("Shaders/DefUberShader.glsl"),
	m_DiffuseColor(glm::vec3(0.65f, 0.65f, 0.65f)),
	m_SpecularColor(glm::vec3(1.0f, 1.0f, 1.0f))
{
	m_LayoutFlags = VertexFlags::POSITION | VertexFlags::NORMAL | VertexFlags::TANGENT | VertexFlags::TEXCOORD;
}
GbufferMaterial::~GbufferMaterial()
{
}

void GbufferMaterial::LoadTextures()
{
	TextureLoader* pTL = ContentManager::GetLoader<TextureLoader, TextureData>();
	STATE->SetShader(m_Shader);
	m_uUseDifTex = glGetUniformLocation(m_Shader->GetProgram(), "useDifTex");
	glUniform1i(m_uUseDifTex, m_UseDifTex);
	m_uUseNormTex = glGetUniformLocation(m_Shader->GetProgram(), "useNormTex");
	glUniform1i(m_uUseNormTex, m_UseNormTex);
	m_uUseSpecTex = glGetUniformLocation(m_Shader->GetProgram(), "useSpecTex");
	glUniform1i(m_uUseSpecTex, m_UseSpecTex);

	pTL->UseSrgb(true);
	if (m_UseDifTex)
	{
		m_TexDiffuse = ContentManager::Load<TextureData>(m_TexDiffusePath);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texDiffuse"), 0);
	}
	if (m_UseSpecTex)
	{
		m_TexSpec = ContentManager::Load<TextureData>(m_TexSpecPath);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texSpecular"), 2);
	}
	pTL->UseSrgb(false);
	if (m_UseNormTex)
	{
		m_TexNorm = ContentManager::Load<TextureData>(m_TexNormPath);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texNormal"), 1);
	}
	m_OutdatedTextureData = false;
}

void GbufferMaterial::AccessShaderAttributes()
{
	m_uDifCol = glGetUniformLocation(m_Shader->GetProgram(), "diffuseColor");
	m_uSpecCol = glGetUniformLocation(m_Shader->GetProgram(), "specularColor");
	m_uSpecPow = glGetUniformLocation(m_Shader->GetProgram(), "specularPower");
}

void GbufferMaterial::UploadDerivedVariables()
{
	//Bind active textures
	if (m_OutdatedTextureData)LoadTextures();
	if (m_UseDifTex)
	{
		STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_TexDiffuse->GetHandle());
	}
	if (m_UseNormTex)
	{
		STATE->LazyBindTexture(1, GL_TEXTURE_2D, m_TexNorm->GetHandle());
	}
	if (m_UseSpecTex)
	{
		STATE->LazyBindTexture(2, GL_TEXTURE_2D, m_TexSpec->GetHandle());
	}
	//Upload uniforms
	glUniform3f(m_uDifCol, m_DiffuseColor.x, m_DiffuseColor.y, m_DiffuseColor.z);
	glUniform3f(m_uSpecCol, m_SpecularColor.x, m_SpecularColor.y, m_SpecularColor.z);
	glUniform1f(m_uSpecPow, m_SpecularPower);
}