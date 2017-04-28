#include "stdafx.hpp"
#include "HdrLoader.hpp"

#include <FreeImage.h>

#include "../Graphics/ShaderData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"
#include "../GraphicsHelper/RenderPipeline.hpp"
#include "../GraphicsHelper/RenderState.hpp"

HdrLoader::HdrLoader()
{
}

HdrLoader::~HdrLoader()
{
}

HDRMap* HdrLoader::LoadContent(const std::string& assetFile)
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//load equirectangular texture
	//****************************
	GLuint hdrTexture;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(assetFile.c_str(), 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(assetFile.c_str());
	if (fif == FIF_UNKNOWN)
		return nullptr;

	FIBITMAP *dib(0);
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, assetFile.c_str());
	if (dib)
	{
		//not flipping because free image already flips automatically --maybe skybox is wrong way around and also flipped in shaders?
		FIBITMAP *pImage = FreeImage_ConvertToType(dib, FIT_RGBF);

		unsigned int width = FreeImage_GetWidth(pImage);
		unsigned int height = FreeImage_GetHeight(pImage);
		BYTE* data = FreeImage_GetBits(pImage);
		if ((data == 0) || (width == 0) || (height == 0))
		{
			cout << "  . . . FAILED! " << endl;
			return nullptr;
		}

		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);

		FreeImage_Unload(dib);
		FreeImage_Unload(pImage);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
		return nullptr;
	}

	//initialize rendering to cubemap
	//*******************************

	//Create framebuffer
	GLuint captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_CubemapRes, m_CubemapRes);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//Preallocate memory for cubemap
	GLuint envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_CubemapRes, m_CubemapRes, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Create matrices for rendering to texture
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	//Get the shader
	auto equiCubeShader = ContentManager::Load<ShaderData>("Shaders/FwdEquiCubeShader.glsl");

	// convert HDR equirectangular environment map to cubemap equivalent
	glUseProgram(equiCubeShader->GetProgram());
	glUniform1i(glGetUniformLocation(equiCubeShader->GetProgram(), "equirectangularMap"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glUniformMatrix4fv(glGetUniformLocation(equiCubeShader->GetProgram(), "projection"), 1, GL_FALSE, glm::value_ptr(captureProjection));

	//render the cube
	//***************

	RenderPipeline::GetInstance()->GetState()->SetViewport(glm::ivec2(0), glm::ivec2(m_CubemapRes));
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(equiCubeShader->GetProgram(), "view"), 1, GL_FALSE, glm::value_ptr(captureViews[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	//setup for convoluted irradiance cubemap
	//***************************************

	//texture
	GLuint irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_IrradianceRes, m_IrradianceRes, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_IrradianceRes, m_IrradianceRes);

	//shader
	auto irradianceShader = ContentManager::Load<ShaderData>("Shaders/FwdConvIrradianceShader.glsl");

	glUseProgram(irradianceShader->GetProgram());
	glUniform1i(glGetUniformLocation(irradianceShader->GetProgram(), "environmentMap"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glUniformMatrix4fv(glGetUniformLocation(irradianceShader->GetProgram(), "projection"), 1, GL_FALSE, glm::value_ptr(captureProjection));

	//render irradiance cubemap
	//*************************

	RenderPipeline::GetInstance()->GetState()->SetViewport(glm::ivec2(0), glm::ivec2(m_IrradianceRes));
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(irradianceShader->GetProgram(), "view"), 1, GL_FALSE, glm::value_ptr(captureViews[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//setup radiance
	//**************
	GLuint radianceMap;
	glGenTextures(1, &radianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, radianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_RadianceRes, m_RadianceRes, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP); 

	//Shader
	auto radianceShader = ContentManager::Load<ShaderData>("Shaders/FwdConvRadianceShader.glsl");

	glUseProgram(radianceShader->GetProgram());
	glUniform1i(glGetUniformLocation(radianceShader->GetProgram(), "environmentMap"), 0);
	glUniform1f(glGetUniformLocation(radianceShader->GetProgram(), "resolution"), (GLfloat)m_RadianceRes);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glUniformMatrix4fv(glGetUniformLocation(radianceShader->GetProgram(), "projection"), 1, GL_FALSE, glm::value_ptr(captureProjection));
	auto roughnessUniformLoc = glGetUniformLocation(radianceShader->GetProgram(), "roughness");

	//render radiance
	//***************
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = (unsigned int)std::log2(m_RadianceRes)-2;//at least 4x4
	for (unsigned int mip = 0; mip < maxMipLevels+1; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = (unsigned int)(m_RadianceRes * std::pow(0.5, mip));
		unsigned int mipHeight = (unsigned int)(m_RadianceRes * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		RenderPipeline::GetInstance()->GetState()->SetViewport(glm::ivec2(0), glm::ivec2(mipWidth, mipHeight));

		float roughness = (float)mip / (float)(maxMipLevels);
		glUniform1f(roughnessUniformLoc, roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(glGetUniformLocation(radianceShader->GetProgram(), "view"), 1, GL_FALSE, glm::value_ptr(captureViews[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, radianceMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//setup BRDF look up table
	//************************
	//Should probably be moved to a different place and made global for future support of reflection probes

	GLuint brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);
	//Shader
	auto brdfShader = ContentManager::Load<ShaderData>("Shaders/FwdBrdfLutShader.glsl");

	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, m_BrdfLutRes, m_BrdfLutRes, 0, GL_RG, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_BrdfLutRes, m_BrdfLutRes);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	RenderPipeline::GetInstance()->GetState()->SetViewport(glm::ivec2(0), glm::ivec2(m_BrdfLutRes));
	glUseProgram(brdfShader->GetProgram());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Reset render settings and return generated texture
	//*************************************************
	glBindFramebuffer(GL_FRAMEBUFFER, 0);//need to set it back later
	RenderPipeline::GetInstance()->GetState()->SetViewport(glm::ivec2(0), glm::ivec2(SETTINGS->Window.Width, SETTINGS->Window.Height));
	glDeleteTextures(1, &hdrTexture);

	return new HDRMap(envCubemap, irradianceMap, radianceMap, brdfLUTTexture, m_CubemapRes, m_CubemapRes, maxMipLevels);
}

void HdrLoader::Destroy(HDRMap* objToDestroy)
{
	if (!(objToDestroy == nullptr))
	{
		delete objToDestroy;
		objToDestroy = nullptr;
	}
}