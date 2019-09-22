#include "stdafx.h"
#include "PbrPrefilter.h"

#include "PrimitiveRenderer.h"

#include <glad/glad.h>

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
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	ivec2 logPos = Logger::GetCursorPosition();
	LOG("Precalculating PBR BRDF LUT . . .");
	//setup BRDF look up table
	//************************
	//Create framebuffer
	GLuint captureFBO, captureRBO;
	api->GenFramebuffers(1, &captureFBO);
	api->GenRenderBuffers(1, &captureRBO);

	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(GL_DEPTH_COMPONENT24, ivec2(resolution));
	api->LinkRenderbufferToFbo(GL_DEPTH_ATTACHMENT, captureRBO);
	//Shader
	api->SetShader(ResourceManager::Instance()->GetAssetData<ShaderData>("FwdBrdfLutShader.glsl"_hash).get());

	m_LUT = new TextureData(resolution, resolution, GL_RG16F, GL_RG, GL_FLOAT);
	m_LUT->Build();
	TextureParameters params(false);
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
	m_LUT->SetParameters(params);

	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(GL_DEPTH_COMPONENT24, ivec2(resolution));
	api->LinkTextureToFbo2D(0, m_LUT->GetHandle(), 0);

	api->SetViewport(ivec2(0), ivec2(resolution));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	//Reset render settings and return generated texture
	//*************************************************
	api->BindFramebuffer(0);
	api->BindRenderbuffer(0);
	api->SetViewport(ivec2(0), Config::GetInstance()->GetWindow().Dimensions);

	api->DeleteRenderBuffers(1, &captureRBO);
	api->DeleteFramebuffers(1, &captureFBO);
	LOG("Precalculating PBR BRDF LUT . . . . . . DONE", Info, false, logPos);
}

void PbrPrefilter::PrefilterCube(TextureData const* const source, 
	TextureData*& irradiance, 
	TextureData*& radiance, 
	int32 const resolution, 
	int32 const irradianceRes, 
	int32 const radianceRes)
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//setup for convoluted irradiance cubemap
	//***************************************
	mat4 captureProjection = CubeCaptureProjection();
	std::vector<mat4> captureViews = CubeCaptureViews();

	//Create framebuffer
	GLuint captureFBO, captureRBO;
	api->GenFramebuffers(1, &captureFBO);
	api->GenRenderBuffers(1, &captureRBO);

	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(GL_DEPTH_COMPONENT24, ivec2(resolution));
	api->LinkRenderbufferToFbo(GL_DEPTH_ATTACHMENT, captureRBO);

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
	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(GL_DEPTH_COMPONENT24, ivec2(irradianceRes));

	//shader
	AssetPtr<ShaderData> irradianceShader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdConvIrradianceShader.glsl"_hash);

	api->SetShader(irradianceShader.get());
	irradianceShader->Upload("environmentMap"_hash, 0);
	api->LazyBindTexture(0, E_TextureType::CubeMap, source->GetHandle());
	irradianceShader->Upload("projection"_hash, captureProjection);

	//render irradiance cubemap
	//*************************

	api->SetViewport(ivec2(0), ivec2(irradianceRes));
	api->BindFramebuffer(captureFBO);
	for (uint8 face = 0; face < 6; ++face)
	{
		irradianceShader->Upload("view"_hash, captureViews[face]);
		api->LinkCubeMapFaceToFbo2D(face, irradiance->GetHandle(), 0);
		api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	api->BindFramebuffer(0);

	//setup radiance
	//**************
	radiance = new TextureData(E_TextureType::CubeMap, radianceRes, radianceRes);
	radiance->Build();
	params.genMipMaps = true;
	radiance->SetParameters(params);

	//Shader
	AssetPtr<ShaderData> radianceShader = ResourceManager::Instance()->GetAssetData<ShaderData>("FwdConvRadianceShader.glsl"_hash);

	api->SetShader(radianceShader.get());
	radianceShader->Upload("environmentMap"_hash, 0);
	radianceShader->Upload("resolution"_hash, static_cast<float>(radianceRes));
	api->LazyBindTexture(0, E_TextureType::CubeMap, source->GetHandle());
	radianceShader->Upload("projection"_hash, captureProjection);

	//render radiance
	//***************
	api->BindFramebuffer(captureFBO);
	uint32 maxMipLevels = (uint32)std::log2(radianceRes) - 2;//at least 4x4
	for (uint32 mip = 0; mip < maxMipLevels + 1; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		uint32 mipWidth = (uint32)(radianceRes * std::pow(0.5, mip));
		uint32 mipHeight = (uint32)(radianceRes * std::pow(0.5, mip));
		api->BindRenderbuffer(captureRBO);
		api->SetRenderbufferStorage(GL_DEPTH_COMPONENT24, ivec2(mipWidth, mipHeight));
		api->SetViewport(ivec2(0), ivec2(mipWidth, mipHeight));

		float roughness = (float)mip / (float)(maxMipLevels);
		radianceShader->Upload("roughness"_hash, roughness);
		for (uint8 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			radianceShader->Upload("view"_hash, captureViews[faceIdx]);
			api->LinkCubeMapFaceToFbo2D(faceIdx, radiance->GetHandle(), mip);

			api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);
			PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
		}
	}

	//Reset render settings and return generated texture
	//*************************************************
	api->BindTexture(E_TextureType::Texture2D, 0);
	api->BindFramebuffer(0);
	api->SetViewport(ivec2(0), Config::GetInstance()->GetWindow().Dimensions);

	api->DeleteRenderBuffers(1, &captureRBO);
	api->DeleteFramebuffers(1, &captureFBO);
}

TextureData* PbrPrefilter::GetLUT()
{
	if (m_LUT == nullptr)
	{
		Precompute(512);
	}

	return m_LUT;
}
