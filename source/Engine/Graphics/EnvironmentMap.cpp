#include "stdafx.h"
#include "EnvironmentMap.h"

#include "TextureData.h"
#include "Shader.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/GraphicsHelper/PrimitiveRenderer.h>


CubeMap* EquirectangularToCubeMap(TextureData* pEqui, int32 resolution)
{
	//Create framebuffer
	GLuint captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//Preallocate memory for cubemap
	GLuint envCubemap;
	glGenTextures(1, &envCubemap);
	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (uint32 i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	STATE->BindFramebuffer(0);

	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);// #note might need to be rebound later
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	int32 mipNum = 1 + (int32)floor(log10((float)resolution) / log10(2.0));

	STATE->BindTexture(GL_TEXTURE_2D, 0);
	STATE->SetViewport(ivec2(0), WINDOW.Dimensions);

	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);

	return new CubeMap(envCubemap, resolution, resolution, mipNum);
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

