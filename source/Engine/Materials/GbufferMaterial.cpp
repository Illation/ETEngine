#include "stdafx.h"
#include "GbufferMaterial.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/MeshFilter.h>


GbufferMaterial::GbufferMaterial() :
	Material("Shaders/DefUberShader.glsl"),
	m_DiffuseColor(vec3(0.65f, 0.65f, 0.65f)),
	m_SpecularColor(vec3(1.0f, 1.0f, 1.0f))
{
	m_LayoutFlags = VertexFlags::POSITION | VertexFlags::NORMAL | VertexFlags::TANGENT | VertexFlags::TEXCOORD;
}

void GbufferMaterial::SetDiffuseTexture(T_Hash const id)
{
	m_TexDiffuseAsset = id;
	m_OutdatedTextureData = true;
}

void GbufferMaterial::SetNormalTexture(T_Hash const id)
{
	m_TexNormAsset = id;
	m_OutdatedTextureData = true;
}

void GbufferMaterial::SetSpecularTexture(T_Hash const id)
{
	m_TexSpecAsset = id;
	m_OutdatedTextureData = true;
}

void GbufferMaterial::LoadTextures()
{
	STATE->SetShader(m_Shader.get());
	m_uUseDifTex = glGetUniformLocation(m_Shader->GetProgram(), "useDifTex");
	glUniform1i(m_uUseDifTex, m_TexDiffuseAsset != 0u);
	m_uUseNormTex = glGetUniformLocation(m_Shader->GetProgram(), "useNormTex");
	glUniform1i(m_uUseNormTex, m_TexNormAsset != 0u);
	m_uUseSpecTex = glGetUniformLocation(m_Shader->GetProgram(), "useSpecTex");
	glUniform1i(m_uUseSpecTex, m_TexSpecAsset != 0u);

	if (m_TexDiffuseAsset != 0u)
	{
		m_TexDiffuse = ResourceManager::GetInstance()->GetAssetData<TextureData>(m_TexDiffuseAsset);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texDiffuse"), 0);
	}
	else
	{
		m_TexDiffuse = nullptr;
	}

	if (m_TexSpecAsset != 0u)
	{
		m_TexSpec = ResourceManager::GetInstance()->GetAssetData<TextureData>(m_TexSpecAsset);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texSpecular"), 2);
	}
	else
	{
		m_TexSpec = nullptr;
	}

	if (m_TexNormAsset != 0u)
	{
		m_TexNorm = ResourceManager::GetInstance()->GetAssetData<TextureData>(m_TexNormAsset);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texNormal"), 1);
	}
	else
	{
		m_TexNorm = nullptr;
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
	if (m_TexDiffuse != nullptr)
	{
		STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_TexDiffuse->GetHandle());
	}
	if (m_TexNorm != nullptr)
	{
		STATE->LazyBindTexture(1, GL_TEXTURE_2D, m_TexNorm->GetHandle());
	}
	if (m_TexSpec != nullptr)
	{
		STATE->LazyBindTexture(2, GL_TEXTURE_2D, m_TexSpec->GetHandle());
	}
	//Upload uniforms
	glUniform3f(m_uDifCol, m_DiffuseColor.x, m_DiffuseColor.y, m_DiffuseColor.z);
	glUniform3f(m_uSpecCol, m_SpecularColor.x, m_SpecularColor.y, m_SpecularColor.z);
	glUniform1f(m_uSpecPow, m_SpecularPower);
}