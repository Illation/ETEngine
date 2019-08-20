#include "stdafx.h"
#include "EnvironmentMap.h"

#include "Shader.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/GraphicsHelper/PrimitiveRenderer.h>


TextureData* EquirectangularToCubeMap(TextureData const* const pEqui, int32 const resolution)
{
	//Create framebuffer
	uint32 captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//Preallocate memory for cubemap
	TextureData* const envCubeMap = new TextureData(E_TextureType::CubeMap, resolution, resolution);
	envCubeMap->Build();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
	params.wrapR = E_TextureWrapMode::ClampToEdge;

	envCubeMap->SetParameters(params);

	std::vector<mat4> captureViews = CubeCaptureViews();

	//Get the shader
	AssetPtr<ShaderData> equiCubeShader = ResourceManager::GetInstance()->GetAssetData<ShaderData>("FwdEquiCubeShader.glsl"_hash);

	// convert HDR equirectangular environment map to cubemap equivalent
	STATE->SetShader(equiCubeShader.get());
	glUniform1i(glGetUniformLocation(equiCubeShader->GetProgram(), "equirectangularMap"), 0);
	STATE->LazyBindTexture(0, GL_TEXTURE_2D, pEqui->GetHandle());
	glUniformMatrix4fv(glGetUniformLocation(equiCubeShader->GetProgram(), "projection"), 1, GL_FALSE, etm::valuePtr(CubeCaptureProjection()));

	//render the cube
	//***************

	STATE->SetViewport(ivec2(0), ivec2(resolution));
	STATE->BindFramebuffer(captureFBO);
	for (uint32 i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(equiCubeShader->GetProgram(), "view"), 1, GL_FALSE, etm::valuePtr(captureViews[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap->GetHandle(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	STATE->BindFramebuffer(0);

	params.genMipMaps = true;
	envCubeMap->SetParameters(params);

	STATE->BindTexture(GL_TEXTURE_2D, 0);
	STATE->SetViewport(ivec2(0), WINDOW.Dimensions);

	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);

	return envCubeMap;
}

mat4 CubeCaptureProjection()
{
	return etm::perspective(etm::radians(90.0f), 1.0f, 0.1f, 10.0f);
}

std::vector<mat4> CubeCaptureViews()
{
	std::vector<mat4> ret;
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));
	return ret;
}

HDRMap::HDRMap(TextureData* map, TextureData* irradiance, TextureData* radiance, int32 width, int32 height, int32 numMipMaps) 
	: m_Map(map)
	, m_Irradiance(irradiance)
	, m_Radiance(radiance)
	, m_Width(width)
	, m_Height(height)
	, m_NumMipMaps(numMipMaps)
{ }

HDRMap::~HDRMap()
{
	delete m_Map;
	delete m_Irradiance;
	delete m_Radiance;
}
