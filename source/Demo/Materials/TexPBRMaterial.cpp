#include "stdafx.h"
#include "TexPBRMaterial.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/MeshFilter.h>


TexPBRMaterial::TexPBRMaterial(T_Hash const bcId, T_Hash const roughId, T_Hash const metalId, T_Hash const aoId, T_Hash const normId) 
	: Material("Shaders/DefPBRMetShader.glsl")
	, m_BaseColorId(bcId)
	, m_RoughnessId(roughId)
	, m_MetalnessId(metalId)
	, m_AoId(aoId)
	, m_NormalId(normId)
{ }

void TexPBRMaterial::LoadTextures()
{
	STATE->SetShader(m_Shader.get());
	
	m_TexBaseColor = ResourceManager::GetInstance()->GetAssetData<TextureData>(m_BaseColorId);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texBaseColor"), 0);

	m_TexRoughness = ResourceManager::GetInstance()->GetAssetData<TextureData>(m_RoughnessId);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texRoughness"), 1);

	m_TexMetalness = ResourceManager::GetInstance()->GetAssetData<TextureData>(m_MetalnessId);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texMetalness"), 2);

	m_TexAO = ResourceManager::GetInstance()->GetAssetData<TextureData>(m_AoId);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "texAO"), 3);

	m_TexNorm = ResourceManager::GetInstance()->GetAssetData<TextureData>(m_NormalId);
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
	if (m_OutdatedTextureData)
	{
		LoadTextures();
	}

	STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_TexBaseColor->GetHandle());
	STATE->LazyBindTexture(1, GL_TEXTURE_2D, m_TexRoughness->GetHandle());
	STATE->LazyBindTexture(2, GL_TEXTURE_2D, m_TexMetalness->GetHandle());
	STATE->LazyBindTexture(3, GL_TEXTURE_2D, m_TexAO->GetHandle());
	STATE->LazyBindTexture(4, GL_TEXTURE_2D, m_TexNorm->GetHandle());

	//Upload uniforms
	glUniform1f(m_uSpecular, m_Specular);
}