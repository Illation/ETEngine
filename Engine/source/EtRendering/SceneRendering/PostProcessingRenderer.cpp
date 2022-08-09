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
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DeleteRenderBuffers(1, &m_CollectRBO);
	delete m_CollectTex; m_CollectTex = nullptr;
	device->DeleteFramebuffers(1, &m_CollectFBO);

	delete m_ColorBuffers[0]; m_ColorBuffers[0] = nullptr;
	delete m_ColorBuffers[1]; m_ColorBuffers[1] = nullptr;
	device->DeleteFramebuffers(1, &m_HDRoutFBO);

	delete m_PingPongTexture[0]; m_PingPongTexture[0] = nullptr;
	delete m_PingPongTexture[1]; m_PingPongTexture[1] = nullptr;
	device->DeleteFramebuffers(2, &(m_PingPongFBO[0]));

	for(uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		delete m_DownSampleTexture[i]; m_DownSampleTexture[i] = nullptr;
		delete m_DownPingPongTexture[i]; m_DownPingPongTexture[i] = nullptr;
	}
	device->DeleteFramebuffers(NUM_BLOOM_DOWNSAMPLES, &(m_DownSampleFBO[0]));
	device->DeleteFramebuffers(NUM_BLOOM_DOWNSAMPLES, &(m_DownPingPongFBO[0]));
}

void PostProcessingRenderer::Initialize()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	//Load and compile Shaders
	m_pDownsampleShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostDownsample.glsl"));
	m_pGaussianShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostGaussian.glsl"));
	m_pPostProcShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostProcessing.glsl"));
	m_pFXAAShader = core::ResourceManager::Instance()->GetAssetData<rhi::ShaderData>(core::HashString("Shaders/PostFXAA.glsl"));

	GenerateFramebuffers();
}

void PostProcessingRenderer::GenerateFramebuffers()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	rhi::TextureParameters params(false);
	params.minFilter = rhi::E_TextureFilterMode::Linear;
	params.magFilter = rhi::E_TextureFilterMode::Linear;
	params.wrapS = rhi::E_TextureWrapMode::ClampToEdge;
	params.wrapT = rhi::E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	device->GenFramebuffers(1, &m_CollectFBO);
	device->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, dim);
	m_CollectTex->AllocateStorage();
	m_CollectTex->SetParameters(params);
	//Render Buffer for depth and stencil
	device->GenRenderBuffers(1, &m_CollectRBO);
	device->BindRenderbuffer(m_CollectRBO);
	device->SetRenderbufferStorage(rhi::E_RenderBufferFormat::Depth24_Stencil8, dim);
	device->LinkRenderbufferToFbo(rhi::E_RenderBufferFormat::Depth24_Stencil8, m_CollectRBO);

	device->LinkTextureToFbo2D(0, m_CollectTex->GetLocation(), 0);

	//Generate textures for the hdr fbo to output into
	device->GenFramebuffers(1, &m_HDRoutFBO);
	device->BindFramebuffer(m_HDRoutFBO);
	for(uint32 i = 0; i < 2; i++)
	{
		m_ColorBuffers[i] = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, dim);
		m_ColorBuffers[i]->AllocateStorage();
		m_ColorBuffers[i]->SetParameters(params, true);
		// attach texture to framebuffer
		device->LinkTextureToFbo2D(i, m_ColorBuffers[i]->GetLocation(), 0);
	}
	//mrt
	device->SetDrawBufferCount(2);

	//Generate framebuffers for downsampling
	device->GenFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownSampleFBO);
	device->GenFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownPingPongFBO);
	for(uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		float resMult = 1.f / (float)std::pow(2, i + 1);
		ivec2 res = math::vecCast<int32>(math::vecCast<float>(dim) * resMult);

		device->BindFramebuffer( m_DownSampleFBO[i] );
		m_DownSampleTexture[i] = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, res);
		m_DownSampleTexture[i]->AllocateStorage();
		m_DownSampleTexture[i]->SetParameters(params, true);
		device->LinkTextureToFbo2D(0, m_DownSampleTexture[i]->GetLocation(), 0);

		device->BindFramebuffer( m_DownPingPongFBO[i] );
		m_DownPingPongTexture[i] = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, res);
		m_DownPingPongTexture[i]->AllocateStorage();
		m_DownPingPongTexture[i]->SetParameters(params, true);
		device->LinkTextureToFbo2D(0, m_DownPingPongTexture[i]->GetLocation(), 0);
	}

	//Generate framebuffers and textures for gaussian ping pong
	device->GenFramebuffers( 2, m_PingPongFBO );
	for(uint32 i = 0; i < 2; i++)
	{
		device->BindFramebuffer( m_PingPongFBO[i] );
		m_PingPongTexture[i] = new rhi::TextureData(rhi::E_ColorFormat::RGB16f, dim);
		m_PingPongTexture[i]->AllocateStorage();
		m_PingPongTexture[i]->SetParameters(params, true);
		device->LinkTextureToFbo2D(0, m_PingPongTexture[i]->GetLocation(), 0);
	}

	if (!(device->IsFramebufferComplete()))
	{
		ET_LOG_E(ET_CTX_RENDER, "Framebuffer::Initialize > FAILED!");
	}
}

void PostProcessingRenderer::EnableInput()
{
	rhi::ContextHolder::GetRenderDevice()->BindFramebuffer(m_CollectFBO);
}

void PostProcessingRenderer::Draw(rhi::T_FbLoc const FBO, PostProcessingSettings const& settings, std::function<void(rhi::T_FbLoc const)>& onDrawOverlaysFn)
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();
	render::GraphicsSettings const& graphicsSettings = RenderingSystems::Instance()->GetGraphicsSettings();

	device->DebugPushGroup("generate bloom");

	device->SetDepthEnabled(false);
	//get glow
	device->BindFramebuffer(m_HDRoutFBO);
	device->SetShader(m_pDownsampleShader.get());
	m_pDownsampleShader->Upload("texColor"_hash, static_cast<rhi::TextureData const*>(m_CollectTex));
	m_pDownsampleShader->Upload("threshold"_hash, settings.bloomThreshold);
	rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
	//downsample glow
	device->DebugPushGroup("downsample glow");
	for (uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		device->DebugPushGroup(FS("downsample pass %u", i));
		if (i > 0)
		{
			device->SetShader(m_pDownsampleShader.get());
		}

		float resMult = 1.f / (float)std::pow(2, i + 1);
		device->SetViewport(ivec2(0), math::vecCast<int32>(math::vecCast<float>(dim) * resMult));
		device->BindFramebuffer(m_DownSampleFBO[i]);
		if (i > 0)
		{
			m_pDownsampleShader->Upload("texColor"_hash, static_cast<rhi::TextureData const*>(m_DownSampleTexture[i - 1]));
		}

		m_pDownsampleShader->Upload("threshold"_hash, settings.bloomThreshold);
		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

		//blur downsampled
		device->DebugPushGroup("blur");
		device->SetShader(m_pGaussianShader.get());
		for (uint32 sample = 0; sample < static_cast<uint32>(graphicsSettings.NumBlurPasses * 2); sample++)
		{
			// #todo: needs custom ping pong buffer, buffers textures are wrong size
			bool horizontal = sample % 2 == 0;
			//output is the current framebuffer, or on the last item the framebuffer of the downsample texture
			device->BindFramebuffer(horizontal ? m_DownPingPongFBO[i] : m_DownSampleFBO[i]);
			//input is previous framebuffers texture, or on first item the result of downsampling
			m_pGaussianShader->Upload("image"_hash, static_cast<rhi::TextureData const*>(horizontal ? m_DownSampleTexture[i] : m_DownPingPongTexture[i]));
			m_pGaussianShader->Upload("horizontal"_hash, horizontal);
			rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
		}

		device->DebugPopGroup(); // blur
		device->DebugPopGroup(); // downsample pass
	}

	device->DebugPopGroup(); // downsample glow

	device->DebugPushGroup("gaussian blur");
	device->SetViewport(ivec2(0), dim);
	//ping pong gaussian blur
	bool horizontal = true;
	device->SetShader(m_pGaussianShader.get());
	for (uint32 i = 0; i < static_cast<uint32>(graphicsSettings.NumBlurPasses * 2); i++)
	{
		device->BindFramebuffer(m_PingPongFBO[horizontal]);
		m_pGaussianShader->Upload("horizontal"_hash, horizontal);
		m_pGaussianShader->Upload("image"_hash, static_cast<rhi::TextureData const*>((i == 0) ? m_ColorBuffers[1] : m_PingPongTexture[!horizontal]));
		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();
		horizontal = !horizontal;
	}

	device->DebugPopGroup(); // gaussian blur
	device->DebugPopGroup(); // generate bloom

	//combine with hdr result
	device->DebugPushGroup("combine bloom + tonemapping");
	rhi::T_FbLoc currentFb = graphicsSettings.UseFXAA ? m_PingPongFBO[1] : FBO;
	device->BindFramebuffer(currentFb);
	device->SetShader(m_pPostProcShader.get());

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
	device->DebugPopGroup(); // bloom + tonemapping

	// Make sure text and sprites get antialiased by drawing them before FXAA
	onDrawOverlaysFn(currentFb);

	// FXAA
	if (graphicsSettings.UseFXAA)
	{
		device->DebugPushGroup("anti  aliasing");

		device->BindFramebuffer(FBO);

		device->SetShader(m_pFXAAShader.get());
		m_pFXAAShader->Upload("uInverseScreen"_hash, 1.f / math::vecCast<float>(dim));
		m_pFXAAShader->Upload("texColor"_hash, static_cast<rhi::TextureData const*>(m_PingPongTexture[1]));

		rhi::PrimitiveRenderer::Instance().Draw<rhi::primitives::Quad>();

		device->DebugPopGroup(); // anti aliasing
	}
}

void PostProcessingRenderer::ResizeFBTextures()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	ivec2 const dim = rhi::Viewport::GetCurrentViewport()->GetDimensions();

	if(dim.x > m_CollectTex->GetResolution().x || dim.x > m_CollectTex->GetResolution().y)
	{
		//completely regenerate everything
		DeleteFramebuffers();

		//Access shader variables
		device->SetShader(m_pDownsampleShader.get());
		m_pDownsampleShader->Upload("texColor"_hash, 0);

		device->SetShader(m_pGaussianShader.get());
		m_pGaussianShader->Upload("image"_hash, 0);

		device->SetShader(m_pPostProcShader.get());
		m_pPostProcShader->Upload("texColor"_hash, 0);
		for (int32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
		{
			m_pPostProcShader->Upload(GetHash(std::string("texBloom") + std::to_string(i)), i + 1);
		}

		device->SetShader(m_pFXAAShader.get());
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
		//device->BindFramebuffer( m_DownSampleFBO[i] );
		float resMult = 1.f / (float)std::pow( 2, i + 1 );
		device->SetViewport(ivec2(0), math::vecCast<int32>(math::vecCast<float>(dim) * resMult));

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
