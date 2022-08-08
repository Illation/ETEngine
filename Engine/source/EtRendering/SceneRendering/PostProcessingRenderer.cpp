#include "stdafx.h"
#include "PostProcessingRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRHI/GraphicsTypes/Shader.h>
#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/Util/PrimitiveRenderer.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace render {


PostProcessingRenderer::PostProcessingRenderer()
{
}
PostProcessingRenderer::~PostProcessingRenderer()
{
	DeleteFramebuffers();
}

void PostProcessingRenderer::DeleteFramebuffers()
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	api->DeleteRenderBuffers(1, &m_CollectRBO);
	delete m_CollectTex; m_CollectTex = nullptr;
	api->DeleteFramebuffers(1, &m_CollectFBO);

	delete m_ColorBuffers[0]; m_ColorBuffers[0] = nullptr;
	delete m_ColorBuffers[1]; m_ColorBuffers[1] = nullptr;
	api->DeleteFramebuffers(1, &m_HDRoutFBO);

	delete m_PingPongTexture[0]; m_PingPongTexture[0] = nullptr;
	delete m_PingPongTexture[1]; m_PingPongTexture[1] = nullptr;
	api->DeleteFramebuffers(2, &(m_PingPongFBO[0]));

	for(uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		delete m_DownSampleTexture[i]; m_DownSampleTexture[i] = nullptr;
		delete m_DownPingPongTexture[i]; m_DownPingPongTexture[i] = nullptr;
	}
	api->DeleteFramebuffers(NUM_BLOOM_DOWNSAMPLES, &(m_DownSampleFBO[0]));
	api->DeleteFramebuffers(NUM_BLOOM_DOWNSAMPLES, &(m_DownPingPongFBO[0]));
}

void PostProcessingRenderer::Initialize()
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	//Load and compile Shaders
	m_pDownsampleShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostDownsample.glsl"));
	m_pGaussianShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostGaussian.glsl"));
	m_pPostProcShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostProcessing.glsl"));
	m_pFXAAShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostFXAA.glsl"));

	GenerateFramebuffers();
}

void PostProcessingRenderer::GenerateFramebuffers()
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	rhi::TextureParameters params(false);
	params.minFilter = rhi::E_TextureFilterMode::Linear;
	params.magFilter = rhi::E_TextureFilterMode::Linear;
	params.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
	params.wrapT = rhi::E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_CollectFBO);
	api->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, dim);
	m_CollectTex->AllocateStorage();
	m_CollectTex->SetParameters(params);
	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_CollectRBO);
	api->BindRenderbuffer(m_CollectRBO);
	api->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24_Stencil8, dim);
	api->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24_Stencil8, m_CollectRBO);

	api->LinkTextureToFbo2D(0, m_CollectTex->GetLocation(), 0);

	//Generate textures for the hdr fbo to output into
	api->GenFramebuffers(1, &m_HDRoutFBO);
	api->BindFramebuffer(m_HDRoutFBO);
	for(uint32 i = 0; i < 2; i++)
	{
		m_ColorBuffers[i] = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, dim);
		m_ColorBuffers[i]->AllocateStorage();
		m_ColorBuffers[i]->SetParameters(params, true);
		// attach texture to framebuffer
		api->LinkTextureToFbo2D(i, m_ColorBuffers[i]->GetLocation(), 0);
	}
	//mrt
	api->SetDrawBufferCount(2);

	//Generate framebuffers for downsampling
	api->GenFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownSampleFBO);
	api->GenFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownPingPongFBO);
	for(uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		float resMult = 1.f / (float)std::pow(2, i + 1);
		ivec2 res = math::vecCast<int32>(math::vecCast<float>(dim) * resMult);

		api->BindFramebuffer( m_DownSampleFBO[i] );
		m_DownSampleTexture[i] = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, res);
		m_DownSampleTexture[i]->AllocateStorage();
		m_DownSampleTexture[i]->SetParameters(params, true);
		api->LinkTextureToFbo2D(0, m_DownSampleTexture[i]->GetLocation(), 0);

		api->BindFramebuffer( m_DownPingPongFBO[i] );
		m_DownPingPongTexture[i] = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, res);
		m_DownPingPongTexture[i]->AllocateStorage();
		m_DownPingPongTexture[i]->SetParameters(params, true);
		api->LinkTextureToFbo2D(0, m_DownPingPongTexture[i]->GetLocation(), 0);
	}

	//Generate framebuffers and textures for gaussian ping pong
	api->GenFramebuffers( 2, m_PingPongFBO );
	for(uint32 i = 0; i < 2; i++)
	{
		api->BindFramebuffer( m_PingPongFBO[i] );
		m_PingPongTexture[i] = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, dim);
		m_PingPongTexture[i]->AllocateStorage();
		m_PingPongTexture[i]->SetParameters(params, true);
		api->LinkTextureToFbo2D(0, m_PingPongTexture[i]->GetLocation(), 0);
	}

	if (!(api->IsFramebufferComplete()))
	{
		ET_LOG_E(ET_CTX_RENDER, "Framebuffer::Initialize > FAILED!");
	}
}

void PostProcessingRenderer::EnableInput()
{
	rhi::ContextHolder::GetRenderContext()->BindFramebuffer(m_CollectFBO);
}

void PostProcessingRenderer::Draw(rhi::T_FbLoc const FBO, PostProcessingSettings const& settings, std::function<void(rhi::T_FbLoc const)>& onDrawOverlaysFn)
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();
	render::GraphicsSettings const& graphicsSettings = RenderingSystems::Instance()->GetGraphicsSettings();

	api->DebugPushGroup("generate bloom");

	api->SetDepthEnabled(false);
	//get glow
	api->BindFramebuffer(m_HDRoutFBO);
	api->SetShader(m_pDownsampleShader.get());
	m_pDownsampleShader->Upload("texColor"_hash, static_cast<rhi::TextureData const*>(m_CollectTex));
	m_pDownsampleShader->Upload("threshold"_hash, settings.bloomThreshold);
	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
	//downsample glow
	api->DebugPushGroup("downsample glow");
	for (uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		api->DebugPushGroup(FS("downsample pass %u", i));
		if (i > 0)
		{
			api->SetShader(m_pDownsampleShader.get());
		}

		float resMult = 1.f / (float)std::pow(2, i + 1);
		api->SetViewport(ivec2(0), math::vecCast<int32>(math::vecCast<float>(dim) * resMult));
		api->BindFramebuffer(m_DownSampleFBO[i]);
		if (i > 0)
		{
			m_pDownsampleShader->Upload("texColor"_hash, static_cast<rhi::TextureData const*>(m_DownSampleTexture[i - 1]));
		}

		m_pDownsampleShader->Upload("threshold"_hash, settings.bloomThreshold);
		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

		//blur downsampled
		api->DebugPushGroup("blur");
		api->SetShader(m_pGaussianShader.get());
		for (uint32 sample = 0; sample < static_cast<uint32>(graphicsSettings.NumBlurPasses * 2); sample++)
		{
			// #todo: needs custom ping pong buffer, buffers textures are wrong size
			bool horizontal = sample % 2 == 0;
			//output is the current framebuffer, or on the last item the framebuffer of the downsample texture
			api->BindFramebuffer(horizontal ? m_DownPingPongFBO[i] : m_DownSampleFBO[i]);
			//input is previous framebuffers texture, or on first item the result of downsampling
			m_pGaussianShader->Upload("image"_hash, static_cast<rhi::TextureData const*>(horizontal ? m_DownSampleTexture[i] : m_DownPingPongTexture[i]));
			m_pGaussianShader->Upload("horizontal"_hash, horizontal);
			rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
		}

		api->DebugPopGroup(); // blur
		api->DebugPopGroup(); // downsample pass
	}

	api->DebugPopGroup(); // downsample glow

	api->DebugPushGroup("gaussian blur");
	api->SetViewport(ivec2(0), dim);
	//ping pong gaussian blur
	bool horizontal = true;
	api->SetShader(m_pGaussianShader.get());
	for (uint32 i = 0; i < static_cast<uint32>(graphicsSettings.NumBlurPasses * 2); i++)
	{
		api->BindFramebuffer(m_PingPongFBO[horizontal]);
		m_pGaussianShader->Upload("horizontal"_hash, horizontal);
		m_pGaussianShader->Upload("image"_hash, static_cast<rhi::TextureData const*>((i == 0) ? m_ColorBuffers[1] : m_PingPongTexture[!horizontal]));
		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
		horizontal = !horizontal;
	}

	api->DebugPopGroup(); // gaussian blur
	api->DebugPopGroup(); // generate bloom

	//combine with hdr result
	api->DebugPushGroup("combine bloom + tonemapping");
	rhi::T_FbLoc currentFb = graphicsSettings.UseFXAA ? m_PingPongFBO[1] : FBO;
	api->BindFramebuffer(currentFb);
	api->SetShader(m_pPostProcShader.get());

	m_pPostProcShader->Upload("texColor"_hash, static_cast<rhi::TextureData const*>(m_CollectTex));
	m_pPostProcShader->Upload("texBloom0"_hash, static_cast<rhi::TextureData const*>(m_PingPongTexture[0]));
	for (int32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		m_pPostProcShader->Upload(GetHash(FS("texBloom%i", i + 1)), static_cast<rhi::TextureData const*>(m_DownSampleTexture[i]));
	}

	// precalculate some tonemapping settings
	{
		// b = linearStrength
		// c = linearAngle
		// d = toeStrength
		// e = toeNumerator
		// f = toeDenominator

		float const eDivF = settings.toeNumerator / settings.toeDenominator;
		float const cb = settings.linearAngle * settings.linearStrength;
		float const de = settings.toeStrength * settings.toeNumerator;
		float const df = settings.toeStrength * settings.toeDenominator;

		m_pPostProcShader->Upload("uShoulderStrength"_hash, settings.shoulderStrength);
		m_pPostProcShader->Upload("uLinearStrength"_hash, settings.linearStrength);

		m_pPostProcShader->Upload("uEdivF"_hash, eDivF);
		m_pPostProcShader->Upload("uCB"_hash, cb);
		m_pPostProcShader->Upload("uDE"_hash, de);
		m_pPostProcShader->Upload("uDF"_hash, df);

		// apply filmic function to linear white
		{
			float const x = settings.linearWhite;
			float const shoulderX = settings.shoulderStrength * x;
			float const fLinWhite = ((x * (shoulderX + cb) + de) / (x * (shoulderX + settings.linearStrength) + df)) - eDivF;

			m_pPostProcShader->Upload("uLinearWhiteMapped"_hash, fLinWhite);
		}
	}

	m_pPostProcShader->Upload("exposure"_hash, settings.exposure);
	m_pPostProcShader->Upload("gamma"_hash, settings.gamma);
	m_pPostProcShader->Upload("bloomMult"_hash, settings.bloomMult);
	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
	api->DebugPopGroup(); // bloom + tonemapping

	// Make sure text and sprites get antialiased by drawing them before FXAA
	onDrawOverlaysFn(currentFb);

	// FXAA
	if (graphicsSettings.UseFXAA)
	{
		api->DebugPushGroup("anti  aliasing");

		api->BindFramebuffer(FBO);

		api->SetShader(m_pFXAAShader.get());
		m_pFXAAShader->Upload("uInverseScreen"_hash, 1.f / math::vecCast<float>(dim));
		m_pFXAAShader->Upload("texColor"_hash, static_cast<rhi::TextureData const*>(m_PingPongTexture[1]));

		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

		api->DebugPopGroup(); // anti aliasing
	}
}

void PostProcessingRenderer::ResizeFBTextures()
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	if(dim.x > m_CollectTex->GetResolution().x || dim.x > m_CollectTex->GetResolution().y)
	{
		//completely regenerate everything
		DeleteFramebuffers();

		//Access shader variables
		api->SetShader(m_pDownsampleShader.get());
		m_pDownsampleShader->Upload("texColor"_hash, 0);

		api->SetShader(m_pGaussianShader.get());
		m_pGaussianShader->Upload("image"_hash, 0);

		api->SetShader(m_pPostProcShader.get());
		m_pPostProcShader->Upload("texColor"_hash, 0);
		for (int32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
		{
			m_pPostProcShader->Upload(GetHash(std::string("texBloom") + std::to_string(i)), i + 1);
		}

		api->SetShader(m_pFXAAShader.get());
		m_pFXAAShader->Upload("texColor"_hash, 0);

		GenerateFramebuffers();
		return;
	}

	m_CollectTex->Resize(dim);
	for(uint32 i = 0; i < 2; i++)
	{
		m_ColorBuffers[i]->Resize(dim);
	}

	for(uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		//api->BindFramebuffer( m_DownSampleFBO[i] );
		float resMult = 1.f / (float)std::pow( 2, i + 1 );
		api->SetViewport(ivec2(0), math::vecCast<int32>(math::vecCast<float>(dim) * resMult));

		m_DownSampleTexture[i]->Resize(dim);
		m_DownPingPongTexture[i]->Resize(dim);
	}

	for(uint32 i = 0; i < 2; i++)
	{
		m_PingPongTexture[i]->Resize(dim);
	}
}


} // namespace render
} // namespace et
