#include "stdafx.h"
#include "SkyboxMaterial.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/MeshFilter.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/EnvironmentMap.h>
#include <Engine/Components/TransformComponent.h>


SkyboxMaterial::SkyboxMaterial(T_Hash const assetId) 
	: Material("Shaders/FwdSkyboxShader.glsl")
	, m_AssetId(assetId)
{
	m_LayoutFlags = VertexFlags::POSITION;
	m_DrawForward = true;
	m_StandardTransform = false;
}

SkyboxMaterial::~SkyboxMaterial()
{
}

void SkyboxMaterial::LoadTextures()
{
	m_EnvironmentMap = ResourceManager::GetInstance()->GetAssetData<EnvironmentMap>(m_AssetId);
}

void SkyboxMaterial::AccessShaderAttributes()
{
	m_uNumMipMaps = glGetUniformLocation(m_Shader->GetProgram(), "numMipMaps");
	m_uRoughness = glGetUniformLocation(m_Shader->GetProgram(), "roughness");
}

void SkyboxMaterial::UploadDerivedVariables()
{
	STATE->SetShader(m_Shader.get());
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "skybox"), 0);
	STATE->LazyBindTexture(0, GL_TEXTURE_CUBE_MAP, m_EnvironmentMap->GetRadianceHandle());

	glUniform1i(m_uNumMipMaps, m_EnvironmentMap->GetNumMipMaps());
	glUniform1f(m_uRoughness, m_Roughness);

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgram(), "viewProj"), 1, GL_FALSE, etm::valuePtr(CAMERA->GetStatViewProj()));
}