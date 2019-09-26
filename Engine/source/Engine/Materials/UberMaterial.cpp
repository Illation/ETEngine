#include "stdafx.h"
#include "UberMaterial.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>


UberMaterial::UberMaterial() :
	Material("Shaders/DefUberShader.glsl"),
	m_BaseColor(vec3(0.65f, 0.65f, 0.65f)),
	m_Roughness(0.f)
{ }

void UberMaterial::SetBaseColorTexture(T_Hash const id)
{
	m_TexBaseColorAsset = id;
	m_OutdatedTextureData = true;
}

void UberMaterial::SetNormalTexture(T_Hash const id)
{
	m_TexNormalAsset = id;
	m_OutdatedTextureData = true;
}

void UberMaterial::SetMetallicRoughnessTexture(T_Hash const id)
{
	m_TexMetallicRoughnessAsset = id;
	m_OutdatedTextureData = true;
}

void UberMaterial::LoadTextures()
{
	Viewport::GetCurrentApiContext()->SetShader(m_Shader.get());
	m_Shader->Upload("uUseBaseColTex"_hash, (m_TexBaseColorAsset != 0u));
	m_Shader->Upload("uUseNormalTex"_hash, (m_TexNormalAsset != 0u));
	m_Shader->Upload("uUseMetallicRoughnessTex"_hash, (m_TexMetallicRoughnessAsset != 0u));

	if (m_TexBaseColorAsset != 0u)
	{
		m_TexBaseColor = ResourceManager::Instance()->GetAssetData<TextureData>(m_TexBaseColorAsset);
		m_Shader->Upload("uTexBaseColor"_hash, 0);
	}
	else
	{
		m_TexBaseColor = nullptr;
	}

	if (m_TexMetallicRoughnessAsset != 0u)
	{
		m_TexMetallicRoughness = ResourceManager::Instance()->GetAssetData<TextureData>(m_TexMetallicRoughnessAsset);
		m_Shader->Upload("uTexMetallicRoughness"_hash, 2);
	}
	else
	{
		m_TexMetallicRoughness = nullptr;
	}

	if (m_TexNormalAsset != 0u)
	{
		m_TexNormal = ResourceManager::Instance()->GetAssetData<TextureData>(m_TexNormalAsset);
		m_Shader->Upload("uTexNormal"_hash, 1);
	}
	else
	{
		m_TexNormal = nullptr;
	}

	m_OutdatedTextureData = false;
}

void UberMaterial::UploadDerivedVariables()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Bind active textures
	if (m_OutdatedTextureData)
	{
		LoadTextures();
	}

	if (m_TexBaseColor != nullptr)
	{
		api->LazyBindTexture(0, E_TextureType::Texture2D, m_TexBaseColor->GetHandle());
	}

	if (m_TexNormal != nullptr)
	{
		api->LazyBindTexture(1, E_TextureType::Texture2D, m_TexNormal->GetHandle());
	}

	if (m_TexMetallicRoughness != nullptr)
	{
		api->LazyBindTexture(2, E_TextureType::Texture2D, m_TexMetallicRoughness->GetHandle());
	}

	//Upload uniforms
	m_Shader->Upload("uBaseColor"_hash, m_BaseColor);
	m_Shader->Upload("uRoughness"_hash, m_Roughness);
	m_Shader->Upload("uMetallic"_hash, m_Metallic);
}