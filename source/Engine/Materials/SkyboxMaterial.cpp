#include "stdafx.h"
#include "SkyboxMaterial.h"

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/MeshFilter.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/Content/CubeMapLoader.h>
#include <Engine/Content/HdrLoader.h>
#include <Engine/Components/TransformComponent.h>


SkyboxMaterial::SkyboxMaterial(std::string assetFile):
	Material("Shaders/FwdSkyboxShader.glsl"),
	m_AssetFile(assetFile)
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
	auto pLoader = ContentManager::GetLoader<HdrLoader, HDRMap>();
	pLoader->UseSrgb(true);
	m_pHDRMap = ContentManager::Load<HDRMap>(m_AssetFile);
}
void SkyboxMaterial::AccessShaderAttributes()
{
	m_uNumMipMaps = glGetUniformLocation(m_Shader->GetProgram(), "numMipMaps");
	m_uRoughness = glGetUniformLocation(m_Shader->GetProgram(), "roughness");
}
void SkyboxMaterial::UploadDerivedVariables()
{
	STATE->SetShader(m_Shader);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "skybox"), 0);
	STATE->LazyBindTexture(0, GL_TEXTURE_CUBE_MAP, m_pHDRMap->GetRadianceHandle());

	glUniform1i(m_uNumMipMaps, m_pHDRMap->GetNumMipMaps());
	glUniform1f(m_uRoughness, m_Roughness);

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgram(), "viewProj"), 1, GL_FALSE, etm::valuePtr(CAMERA->GetStatViewProj()));
}