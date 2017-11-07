#include "stdafx.hpp"
#include "HdrLoader.hpp"

#include <FreeImage.h>

#include "../Graphics/ShaderData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"

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

		uint32 width = FreeImage_GetWidth(pImage);
		uint32 height = FreeImage_GetHeight(pImage);
		BYTE* data = FreeImage_GetBits(pImage);
		if ((data == 0) || (width == 0) || (height == 0))
		{
			cout << "  . . . FAILED! " << endl;
			return nullptr;
		}

		glGenTextures(1, &hdrTexture);
		STATE->BindTexture(GL_TEXTURE_2D, hdrTexture);
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

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_CubemapRes, m_CubemapRes);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//Preallocate memory for cubemap
	GLuint envCubemap;
	glGenTextures(1, &envCubemap);
	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (uint32 i = 0; i < 6; ++i)
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
	mat4 captureProjection = etm::perspective( etm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	mat4 captureViews[] =
	{
		etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f, 0.0f), vec3(0.0f, -1.0f,  0.0f)),
		etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
		etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
		etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
		etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))
	};

	//Get the shader
	auto equiCubeShader = ContentManager::Load<ShaderData>("Shaders/FwdEquiCubeShader.glsl");

	// convert HDR equirectangular environment map to cubemap equivalent
	STATE->SetShader(equiCubeShader);
	glUniform1i(glGetUniformLocation(equiCubeShader->GetProgram(), "equirectangularMap"), 0);
	STATE->LazyBindTexture(0, GL_TEXTURE_2D, hdrTexture);
	glUniformMatrix4fv(glGetUniformLocation(equiCubeShader->GetProgram(), "projection"), 1, GL_FALSE, etm::valuePtr(captureProjection));

	//render the cube
	//***************

	STATE->SetViewport(ivec2(0), ivec2(m_CubemapRes));
	STATE->BindFramebuffer(captureFBO);
	for (uint32 i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(equiCubeShader->GetProgram(), "view"), 1, GL_FALSE, etm::valuePtr(captureViews[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	STATE->BindFramebuffer(0);

	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	//setup for convoluted irradiance cubemap
	//***************************************

	//texture
	GLuint irradianceMap;
	glGenTextures(1, &irradianceMap);
	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (uint32 i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_IrradianceRes, m_IrradianceRes, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Framebuffer
	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_IrradianceRes, m_IrradianceRes);

	//shader
	auto irradianceShader = ContentManager::Load<ShaderData>("Shaders/FwdConvIrradianceShader.glsl");

	STATE->SetShader(irradianceShader);
	glUniform1i(glGetUniformLocation(irradianceShader->GetProgram(), "environmentMap"), 0);
	STATE->LazyBindTexture(0, GL_TEXTURE_CUBE_MAP, envCubemap);
	glUniformMatrix4fv(glGetUniformLocation(irradianceShader->GetProgram(), "projection"), 1, GL_FALSE, etm::valuePtr(captureProjection));

	//render irradiance cubemap
	//*************************

	STATE->SetViewport(ivec2(0), ivec2(m_IrradianceRes));
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

	STATE->SetShader(radianceShader);
	glUniform1i(glGetUniformLocation(radianceShader->GetProgram(), "environmentMap"), 0);
	glUniform1f(glGetUniformLocation(radianceShader->GetProgram(), "resolution"), (GLfloat)m_RadianceRes);
	STATE->LazyBindTexture(0, GL_TEXTURE_CUBE_MAP, envCubemap);
	glUniformMatrix4fv(glGetUniformLocation(radianceShader->GetProgram(), "projection"), 1, GL_FALSE, etm::valuePtr(captureProjection));
	auto roughnessUniformLoc = glGetUniformLocation(radianceShader->GetProgram(), "roughness");

	//render radiance
	//***************
	STATE->BindFramebuffer(captureFBO);
	uint32 maxMipLevels = (uint32)std::log2(m_RadianceRes)-2;//at least 4x4
	for (uint32 mip = 0; mip < maxMipLevels+1; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		uint32 mipWidth = (uint32)(m_RadianceRes * std::pow(0.5, mip));
		uint32 mipHeight = (uint32)(m_RadianceRes * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		STATE->SetViewport(ivec2(0), ivec2(mipWidth, mipHeight));

		float roughness = (float)mip / (float)(maxMipLevels);
		glUniform1f(roughnessUniformLoc, roughness);
		for (uint32 i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(glGetUniformLocation(radianceShader->GetProgram(), "view"), 1, GL_FALSE, etm::valuePtr(captureViews[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, radianceMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
		}
	}
	STATE->BindFramebuffer(0);

	//setup BRDF look up table
	//************************
	//Should probably be moved to a different place and made global for future support of reflection probes

	GLuint brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);
	//Shader
	auto brdfShader = ContentManager::Load<ShaderData>("Shaders/FwdBrdfLutShader.glsl");

	// pre-allocate enough memory for the LUT texture.
	STATE->BindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, m_BrdfLutRes, m_BrdfLutRes, 0, GL_RG, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_BrdfLutRes, m_BrdfLutRes);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	STATE->SetViewport(ivec2(0), ivec2(m_BrdfLutRes));
	STATE->SetShader(brdfShader);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	//Reset render settings and return generated texture
	//*************************************************
	STATE->BindFramebuffer(0);
	STATE->SetViewport(ivec2(0), WINDOW.Dimensions);
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