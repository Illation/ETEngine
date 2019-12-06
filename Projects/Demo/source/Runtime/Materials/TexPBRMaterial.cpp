#include "stdafx.h"
#include "TexPBRMaterial.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>


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
	Viewport::GetCurrentApiContext()->SetShader(m_Shader.get());
	
	m_TexBaseColor = ResourceManager::Instance()->GetAssetData<TextureData>(m_BaseColorId);
	m_TexRoughness = ResourceManager::Instance()->GetAssetData<TextureData>(m_RoughnessId);
	m_TexMetalness = ResourceManager::Instance()->GetAssetData<TextureData>(m_MetalnessId);
	m_TexAO = ResourceManager::Instance()->GetAssetData<TextureData>(m_AoId);
	m_TexNorm = ResourceManager::Instance()->GetAssetData<TextureData>(m_NormalId);

	m_OutdatedTextureData = false;
}

void TexPBRMaterial::UploadDerivedVariables()
{
	//Bind active textures
	if (m_OutdatedTextureData)
	{
		LoadTextures();
	}

	m_Shader->Upload("texBaseColor"_hash, m_TexBaseColor.get());
	m_Shader->Upload("texRoughness"_hash, m_TexRoughness.get());
	m_Shader->Upload("texMetalness"_hash, m_TexMetalness.get());
	m_Shader->Upload("texAO"_hash, m_TexAO.get());
	m_Shader->Upload("texNormal"_hash, m_TexNorm.get());

	//Upload uniforms
	//m_Shader->Upload("specular"_hash, m_Specular);
}