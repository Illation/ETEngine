#include "stdafx.h"
#include "PbrPrefilter.h"

#include "PrimitiveRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/EnvironmentMap.h>
#include <Engine/Graphics/Shader.h>


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
	STATE->SetShader(ResourceManager::GetInstance()->GetAssetData<ShaderData>("FwdBrdfLutShader.glsl"_hash).get());

	m_LUT = new TextureData(resolution, resolution, GL_RG16F, GL_RG, GL_FLOAT);
	m_LUT->Build();
	TextureParameters params(false);
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
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

void PbrPrefilter::PrefilterCube(TextureData const* const source, 
	TextureData*& irradiance, 
	TextureData*& radiance, 
	int32 const resolution, 
	int32 const irradianceRes, 
	int32 const radianceRes)
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
	irradiance = new TextureData(E_TextureType::CubeMap, irradianceRes, irradianceRes);
	irradiance->Build();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
	params.wrapR = E_TextureWrapMode::ClampToEdge;

	irradiance->SetParameters(params);

	//Framebuffer
	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceRes, irradianceRes);

	//shader
	AssetPtr<ShaderData> irradianceShader = ResourceManager::GetInstance()->GetAssetData<ShaderData>("FwdConvIrradianceShader.glsl"_hash);

	STATE->SetShader(irradianceShader.get());
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance->GetHandle(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	STATE->BindFramebuffer(0);

	//setup radiance
	//**************
	radiance = new TextureData(E_TextureType::CubeMap, radianceRes, radianceRes);
	radiance->Build();
	params.genMipMaps = true;
	radiance->SetParameters(params);

	//Shader
	AssetPtr<ShaderData> radianceShader = ResourceManager::GetInstance()->GetAssetData<ShaderData>("FwdConvRadianceShader.glsl"_hash);

	STATE->SetShader(radianceShader.get());
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
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, radiance->GetHandle(), mip);

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
}

TextureData* PbrPrefilter::GetLUT()
{
	if (!m_LUT)
		Precompute(512);
	return m_LUT;
}
