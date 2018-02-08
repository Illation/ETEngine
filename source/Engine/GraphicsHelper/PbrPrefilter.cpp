#include "stdafx.hpp"
#include "PbrPrefilter.h"
#include "PrimitiveRenderer.hpp"
#include "TextureData.hpp"
#include "ShaderData.hpp"

PbrPrefilter::PbrPrefilter()
{

}

PbrPrefilter::~PbrPrefilter()
{
	delete m_LUT;
	m_LUT = nullptr;
}

void PbrPrefilter::Precompute(int32 resolution)
{
	ivec2 logPos = Logger::GetCursorPosition();
	LOG("Precalculating PBR BRDF LUT . . .");
	//setup BRDF look up table
	//************************
	//Create framebuffer
	GLuint captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO); 
	//Shader
	STATE->SetShader(ContentManager::Load<ShaderData>("Shaders/FwdBrdfLutShader.glsl"));

	m_LUT = new TextureData(resolution, resolution, GL_RG16F, GL_RG, GL_FLOAT);
	m_LUT->Build();
	TextureParameters params(false);
	params.wrapS = GL_CLAMP_TO_EDGE;
	params.wrapT = GL_CLAMP_TO_EDGE;
	m_LUT->SetParameters(params);

	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_LUT->GetHandle(), 0);

	STATE->SetViewport(ivec2(0), ivec2(resolution));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	//Reset render settings and return generated texture
	//*************************************************
	STATE->BindFramebuffer(0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	STATE->SetViewport(ivec2(0), WINDOW.Dimensions);

	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);
	LOG("Precalculating PBR BRDF LUT . . . . . . DONE", Info, false, logPos);
}

void PbrPrefilter::PrefilterCube(CubeMap* source, CubeMap* &irradiance, CubeMap* &radiance, int32 resolution, int32 irradianceRes, int32 radianceRes)
{
	//setup for convoluted irradiance cubemap
	//***************************************
	mat4 captureProjection = CubeCaptureProjection();
	std::vector<mat4> captureViews = CubeCaptureViews();

	//Create framebuffer
	GLuint captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//texture
	GLuint irradianceMap;
	glGenTextures(1, &irradianceMap);
	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (uint32 i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irradianceRes, irradianceRes, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Framebuffer
	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceRes, irradianceRes);

	//shader
	auto irradianceShader = ContentManager::Load<ShaderData>("Shaders/FwdConvIrradianceShader.glsl");

	STATE->SetShader(irradianceShader);
	glUniform1i(glGetUniformLocation(irradianceShader->GetProgram(), "environmentMap"), 0);
	STATE->LazyBindTexture(0, GL_TEXTURE_CUBE_MAP, source->GetHandle());
	glUniformMatrix4fv(glGetUniformLocation(irradianceShader->GetProgram(), "projection"), 1, GL_FALSE, etm::valuePtr(captureProjection));

	//render irradiance cubemap
	//*************************

	STATE->SetViewport(ivec2(0), ivec2(irradianceRes));
	STATE->BindFramebuffer(captureFBO);
	for (uint32 i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(irradianceShader->GetProgram(), "view"), 1, GL_FALSE, etm::valuePtr(captureViews[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	STATE->BindFramebuffer(0);

	//setup radiance
	//**************
	GLuint radianceMap;
	glGenTextures(1, &radianceMap);
	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, radianceMap);
	for (uint32 i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, radianceRes, radianceRes, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	//Shader
	auto radianceShader = ContentManager::Load<ShaderData>("Shaders/FwdConvRadianceShader.glsl");

	STATE->SetShader(radianceShader);
	glUniform1i(glGetUniformLocation(radianceShader->GetProgram(), "environmentMap"), 0);
	glUniform1f(glGetUniformLocation(radianceShader->GetProgram(), "resolution"), (GLfloat)radianceRes);
	STATE->LazyBindTexture(0, GL_TEXTURE_CUBE_MAP, source->GetHandle());
	glUniformMatrix4fv(glGetUniformLocation(radianceShader->GetProgram(), "projection"), 1, GL_FALSE, etm::valuePtr(captureProjection));
	auto roughnessUniformLoc = glGetUniformLocation(radianceShader->GetProgram(), "roughness");
	auto viewLoc = glGetUniformLocation(radianceShader->GetProgram(), "view");

	//render radiance
	//***************
	STATE->BindFramebuffer(captureFBO);
	uint32 maxMipLevels = (uint32)std::log2(radianceRes) - 2;//at least 4x4
	for (uint32 mip = 0; mip < maxMipLevels + 1; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		uint32 mipWidth = (uint32)(radianceRes * std::pow(0.5, mip));
		uint32 mipHeight = (uint32)(radianceRes * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		STATE->SetViewport(ivec2(0), ivec2(mipWidth, mipHeight));

		float roughness = (float)mip / (float)(maxMipLevels);
		glUniform1f(roughnessUniformLoc, roughness);
		for (uint32 i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, etm::valuePtr(captureViews[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, radianceMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
		}
	}

	//Reset render settings and return generated texture
	//*************************************************
	STATE->BindTexture(GL_TEXTURE_2D, 0);
	STATE->BindFramebuffer(0);
	STATE->SetViewport(ivec2(0), WINDOW.Dimensions);

	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);

	radiance = new CubeMap(radianceMap, radianceRes, radianceRes, (int32)maxMipLevels);
	irradiance = new CubeMap(irradianceMap, irradianceRes, irradianceRes, (int32)maxMipLevels);
}

TextureData* PbrPrefilter::GetLUT()
{
	if (!m_LUT)
		Precompute(512);
	return m_LUT;
}
