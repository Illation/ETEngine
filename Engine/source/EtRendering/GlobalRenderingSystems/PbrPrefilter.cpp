#include "stdafx.h"
#include "PbrPrefilter.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/Util/PrimitiveRenderer.h>

#include <EtRendering/GraphicsTypes/EnvironmentMap.h>


namespace et {
namespace render {


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
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	ET_TRACE_I(ET_CTX_RENDER, "Precalculating PBR BRDF LUT . . .");
	//setup BRDF look up table
	//************************
	//Create framebuffer
	rhi::T_FbLoc captureFBO;
	rhi::T_RbLoc captureRBO;
	api->GenFramebuffers(1, &captureFBO);
	api->GenRenderBuffers(1, &captureRBO);

	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24, ivec2(resolution));
	api->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24, captureRBO);
	//Shader
	api->SetShader(core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/FwdBrdfLutShader.glsl")).get());

	m_LUT = new rhi::TextureData(rhi::E_ColorFormat::RG16f, ivec2(resolution));
	m_LUT->AllocateStorage();
	rhi::TextureParameters params(false);
	params.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
	params.wrapT = rhi::E_TextureWrapMode::ClampToEdge;
	m_LUT->SetParameters(params);

	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24, ivec2(resolution));
	api->LinkTextureToFbo2D(0, m_LUT->GetLocation(), 0);

	ivec2 pos, size;
	api->GetViewport(pos, size);

	api->SetViewport(ivec2(0), ivec2(resolution));
	api->Clear(rhi::E_ClearFlag::CF_Color | rhi::E_ClearFlag::CF_Depth);
	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

	//Reset render settings and return generated texture
	//*************************************************
	api->BindFramebuffer(0);
	api->BindRenderbuffer(0);
	api->SetViewport(pos, size);

	api->DeleteRenderBuffers(1, &captureRBO);
	api->DeleteFramebuffers(1, &captureFBO);
	ET_TRACE_V(ET_CTX_RENDER, "Completed precalculating PPBR BRDF LUT");
}

void PbrPrefilter::PrefilterCube(rhi::TextureData const* const source, 
	rhi::TextureData*& irradiance, 
	rhi::TextureData*& radiance, 
	int32 const resolution, 
	int32 const irradianceRes, 
	int32 const radianceRes)
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	//setup for convoluted irradiance cubemap
	//***************************************
	mat4 captureProjection = CubeCaptureProjection();
	std::vector<mat4> captureViews = CubeCaptureViews();

	//Create framebuffer
	rhi::T_FbLoc captureFBO;
	rhi::T_RbLoc captureRBO;
	api->GenFramebuffers(1, &captureFBO);
	api->GenRenderBuffers(1, &captureRBO);

	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24, ivec2(resolution));
	api->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24, captureRBO);

	//texture
	irradiance = new rhi::TextureData(rhi::E_TextureType::CubeMap, rhi::E_ColorFormat::RGB16f, ivec2(irradianceRes));
	irradiance->AllocateStorage();

	rhi::TextureParameters params;
	PopulateCubeTextureParams(params);
	params.genMipMaps = false;

	irradiance->SetParameters(params);

	//Framebuffer
	api->BindFramebuffer(captureFBO);
	api->BindRenderbuffer(captureRBO);
	api->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24, ivec2(irradianceRes));

	//shader
	AssetPtr<rhi::ShaderData> irradianceShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/FwdConvIrradianceShader.glsl"));

	api->SetShader(irradianceShader.get());
	irradianceShader->Upload("environmentMap"_hash, source);
	irradianceShader->Upload("projection"_hash, captureProjection);

	//render irradiance cubemap
	//*************************

	ivec2 pos, size;
	api->GetViewport(pos, size);

	api->SetViewport(ivec2(0), ivec2(irradianceRes));
	api->BindFramebuffer(captureFBO);
	for (uint8 face = 0; face < 6; ++face)
	{
		irradianceShader->Upload("view"_hash, captureViews[face]);
		api->LinkCubeMapFaceToFbo2D(face, irradiance->GetLocation(), 0);
		api->Clear(rhi::E_ClearFlag::CF_Color | rhi::E_ClearFlag::CF_Depth);

		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Cube>();
	}

	api->BindFramebuffer(0);

	//setup radiance
	//**************
	radiance = new rhi::TextureData(rhi::E_TextureType::CubeMap, rhi::E_ColorFormat::RGB16f, ivec2(radianceRes));
	radiance->AllocateStorage();
	params.genMipMaps = true;
	radiance->SetParameters(params);
	radiance->GenerateMipMaps();

	//Shader
	AssetPtr<rhi::ShaderData> radianceShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/FwdConvRadianceShader.glsl"));

	api->SetShader(radianceShader.get());
	radianceShader->Upload("environmentMap"_hash, source);
	radianceShader->Upload("resolution"_hash, static_cast<float>(radianceRes));
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
		api->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24, ivec2(mipWidth, mipHeight));
		api->SetViewport(ivec2(0), ivec2(mipWidth, mipHeight));

		float roughness = (float)mip / (float)(maxMipLevels);
		radianceShader->Upload("roughness"_hash, roughness);
		for (uint8 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			radianceShader->Upload("view"_hash, captureViews[faceIdx]);
			api->LinkCubeMapFaceToFbo2D(faceIdx, radiance->GetLocation(), mip);

			api->Clear(rhi::E_ClearFlag::CF_Color | rhi::E_ClearFlag::CF_Depth);
			rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Cube>();
		}
	}

	//Reset render settings and return generated texture
	//*************************************************
	api->UnbindTexture(source->GetTargetType(), source->GetLocation());
	api->UnbindTexture(radiance->GetTargetType(), radiance->GetLocation());
	api->BindFramebuffer(0);
	api->SetViewport(pos, size);

	api->DeleteRenderBuffers(1, &captureRBO);
	api->DeleteFramebuffers(1, &captureFBO);
}

rhi::TextureData* PbrPrefilter::GetLUT()
{
	if (m_LUT == nullptr)
	{
		Precompute(512);
	}

	return m_LUT;
}

//-----------------------------------------
// PbrPrefilter::PopulateCubeTextureParams
//
// Parameters for environment map cube maps
//
void PbrPrefilter::PopulateCubeTextureParams(rhi::TextureParameters& params)
{
	params.minFilter = rhi::E_TextureFilterMode::Linear;
	params.magFilter = rhi::E_TextureFilterMode::Linear;
	params.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
	params.wrapT = rhi::E_TextureWrapMode::ClampToEdge;
	params.wrapR = rhi::E_TextureWrapMode::ClampToEdge;
	params.genMipMaps = true;
}


} // namespace render
} // namespace et
