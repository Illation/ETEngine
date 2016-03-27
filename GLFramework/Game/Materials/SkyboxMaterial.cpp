#include "stdafx.hpp"
#include "SkyboxMaterial.hpp"

#include "../../Graphics/ShaderData.hpp"
#include "../../Graphics/MeshFilter.hpp"
#include "../../Graphics/TextureData.hpp"

#include "../../Content/CubeMapLoader.hpp"

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
	CubeMapLoader* pCmL = ContentManager::GetLoader<CubeMapLoader, CubeMap>();
	pCmL->UseSrgb(true);
	m_pMap = ContentManager::Load<CubeMap>(m_AssetFile);
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
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_pMap->GetHandle());

	glUniform1i(m_uNumMipMaps, m_pMap->GetNumMipMaps());
	glUniform1f(m_uRoughness, m_Roughness);
}