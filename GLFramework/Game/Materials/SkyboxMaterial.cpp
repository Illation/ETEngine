#include "stdafx.hpp"
#include "SkyboxMaterial.hpp"

#include "../../Graphics/ShaderData.hpp"
#include "../../Graphics/MeshFilter.hpp"
#include "../../Graphics/TextureData.hpp"

#include "../../Content/CubeMapLoader.hpp"
#include "../../Content/HdrLoader.hpp"

SkyboxMaterial::SkyboxMaterial(string assetFile):
	Material("Resources/Shaders/FwdSkyboxShader.glsl"),
	m_AssetFile(assetFile)
{
	m_LayoutFlags = VertexFlags::POSITION;
	m_DrawForward = true;
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
	glUseProgram(m_Shader->GetProgram());
	glUniform1i(glGetUniformLocation(m_Shader->GetProgram(), "skybox"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_pHDRMap->GetHandle());

	glUniform1i(m_uNumMipMaps, 1);
	glUniform1f(m_uRoughness, m_Roughness);
}