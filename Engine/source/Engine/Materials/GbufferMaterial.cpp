#include "stdafx.h"
#include "GbufferMaterial.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>


GbufferMaterial::GbufferMaterial() :
	Material("Shaders/DefUberShader.glsl"),
	m_DiffuseColor(vec3(0.65f, 0.65f, 0.65f)),
	m_SpecularColor(vec3(1.0f, 1.0f, 1.0f))
{ }

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
	Viewport::GetCurrentApiContext()->SetShader(m_Shader.get());
	m_Shader->Upload("useDifTex"_hash, (m_TexDiffuseAsset != 0u));
	m_Shader->Upload("useNormTex"_hash, (m_TexNormAsset != 0u));
	m_Shader->Upload("useSpecTex"_hash, (m_TexSpecAsset != 0u));

	if (m_TexDiffuseAsset != 0u)
	{
		m_TexDiffuse = ResourceManager::Instance()->GetAssetData<TextureData>(m_TexDiffuseAsset);
		m_Shader->Upload("texDiffuse"_hash, 0);
	}
	else
	{
		m_TexDiffuse = nullptr;
	}

	if (m_TexSpecAsset != 0u)
	{
		m_TexSpec = ResourceManager::Instance()->GetAssetData<TextureData>(m_TexSpecAsset);
		m_Shader->Upload("texSpecular"_hash, 2);
	}
	else
	{
		m_TexSpec = nullptr;
	}

	if (m_TexNormAsset != 0u)
	{
		m_TexNorm = ResourceManager::Instance()->GetAssetData<TextureData>(m_TexNormAsset);
		m_Shader->Upload("texNormal"_hash, 1);
	}
	else
	{
		m_TexNorm = nullptr;
	}

	m_OutdatedTextureData = false;
}

void GbufferMaterial::UploadDerivedVariables()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Bind active textures
	if (m_OutdatedTextureData)LoadTextures();
	if (m_TexDiffuse != nullptr)
	{
		api->LazyBindTexture(0, E_TextureType::Texture2D, m_TexDiffuse->GetHandle());
	}
	if (m_TexNorm != nullptr)
	{
		api->LazyBindTexture(1, E_TextureType::Texture2D, m_TexNorm->GetHandle());
	}
	if (m_TexSpec != nullptr)
	{
		api->LazyBindTexture(2, E_TextureType::Texture2D, m_TexSpec->GetHandle());
	}

	//Upload uniforms
	m_Shader->Upload("diffuseColor"_hash, m_DiffuseColor);
	//m_Shader->Upload("specularColor"_hash, m_SpecularColor);
	m_Shader->Upload("specularPower"_hash, m_SpecularPower);
}