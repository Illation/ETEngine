#include "stdafx.h"
#include "TexPBRMaterial.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>


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
	
	m_TexBaseColor = ResourceManager::Instance()->GetAssetData<TextureData>(m_BaseColorId);
	m_Shader->Upload("texBaseColor"_hash, 0);

	m_TexRoughness = ResourceManager::Instance()->GetAssetData<TextureData>(m_RoughnessId);
	m_Shader->Upload("texRoughness"_hash, 1);

	m_TexMetalness = ResourceManager::Instance()->GetAssetData<TextureData>(m_MetalnessId);
	m_Shader->Upload("texMetalness"_hash, 2);

	m_TexAO = ResourceManager::Instance()->GetAssetData<TextureData>(m_AoId);
	m_Shader->Upload("texAO"_hash, 3);

	m_TexNorm = ResourceManager::Instance()->GetAssetData<TextureData>(m_NormalId);
	m_Shader->Upload("texNormal"_hash, 4);

	m_OutdatedTextureData = false;
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
	//m_Shader->Upload("specular"_hash, m_Specular);
}