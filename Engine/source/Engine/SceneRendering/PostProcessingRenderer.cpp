#include "stdafx.h"
#include "PostProcessingRenderer.h"

#include "SceneRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/GlobalRenderingSystems/GlobalRenderingSystems.h>


PostProcessingRenderer::PostProcessingRenderer()
{
}
PostProcessingRenderer::~PostProcessingRenderer()
{
	DeleteFramebuffers();
}

void PostProcessingRenderer::DeleteFramebuffers()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

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
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Load and compile Shaders
	m_pDownsampleShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostDownsample.glsl"_hash);
	m_pGaussianShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostGaussian.glsl"_hash);
	m_pPostProcShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostProcessing.glsl"_hash);
	m_pFXAAShader = ResourceManager::Instance()->GetAssetData<ShaderData>("PostFXAA.glsl"_hash);

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
}

void PostProcessingRenderer::GenerateFramebuffers()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_CollectFBO);
	api->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new TextureData(dim, E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
	m_CollectTex->Build();
	m_CollectTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, m_CollectTex->GetHandle(), 0);
	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_CollectRBO);
	api->BindRenderbuffer(m_CollectRBO);
	api->SetRenderbufferStorage(E_RenderBufferFormat::Depth24_Stencil8, dim);
	api->LinkRenderbufferToFbo(E_RenderBufferFormat::Depth24_Stencil8, m_CollectRBO);

	//Generate textures for the hdr fbo to output into
	api->GenFramebuffers(1, &m_HDRoutFBO);
	api->BindFramebuffer(m_HDRoutFBO);
	for(uint32 i = 0; i < 2; i++)
	{
		m_ColorBuffers[i] = new TextureData(dim, E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
		m_ColorBuffers[i]->Build();
		m_ColorBuffers[i]->SetParameters(params, true);
		// attach texture to framebuffer
		api->LinkTextureToFbo2D(i, m_ColorBuffers[i]->GetHandle(), 0);
	}
	//mrt
	api->SetDrawBufferCount(2);

	//Generate framebuffers for downsampling
	api->GenFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownSampleFBO);
	api->GenFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownPingPongFBO);
	for(uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		float resMult = 1.f / (float)std::pow(2, i + 1);
		ivec2 res = etm::vecCast<int32>(etm::vecCast<float>(dim) * resMult);

		api->BindFramebuffer( m_DownSampleFBO[i] );
		m_DownSampleTexture[i] = new TextureData(res, E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
		m_DownSampleTexture[i]->Build();
		m_DownSampleTexture[i]->SetParameters(params, true);
		api->LinkTextureToFbo2D(0, m_DownSampleTexture[i]->GetHandle(), 0);

		api->BindFramebuffer( m_DownPingPongFBO[i] );
		m_DownPingPongTexture[i] = new TextureData(res, E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
		m_DownPingPongTexture[i]->Build();
		m_DownPingPongTexture[i]->SetParameters(params, true);
		api->LinkTextureToFbo2D(0, m_DownPingPongTexture[i]->GetHandle(), 0);
	}

	//Generate framebuffers and textures for gaussian ping pong
	api->GenFramebuffers( 2, m_PingPongFBO );
	for(uint32 i = 0; i < 2; i++)
	{
		api->BindFramebuffer( m_PingPongFBO[i] );
		m_PingPongTexture[i] = new TextureData(dim, E_ColorFormat::RGB16f, E_ColorFormat::RGB, E_DataType::Float);
		m_PingPongTexture[i]->Build();
		m_PingPongTexture[i]->SetParameters(params, true);
		api->LinkTextureToFbo2D(0, m_PingPongTexture[i]->GetHandle(), 0);
	}

	if (!(api->IsFramebufferComplete()))
	{
		LOG("Framebuffer::Initialize > FAILED!", LogLevel::Error);
	}
}

void PostProcessingRenderer::EnableInput()
{
	Viewport::GetCurrentApiContext()->BindFramebuffer(m_CollectFBO);
}
void PostProcessingRenderer::Draw(T_FbLoc const FBO, const PostProcessingSettings &settings)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();
	Config::Settings::Graphics const& graphicsSettings = Config::GetInstance()->GetGraphics();

	api->SetDepthEnabled(false);
	//get glow
	api->BindFramebuffer(m_HDRoutFBO);
	api->SetShader(m_pDownsampleShader.get());
	api->LazyBindTexture(0, E_TextureType::Texture2D, m_CollectTex->GetHandle());
	m_pDownsampleShader->Upload("threshold"_hash, settings.bloomThreshold);
	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
	//downsample glow
	for (uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		if (i > 0) api->SetShader(m_pDownsampleShader.get());
		float resMult = 1.f / (float)std::pow(2, i + 1);
		api->SetViewport(ivec2(0), etm::vecCast<int32>(etm::vecCast<float>(dim) * resMult));
		api->BindFramebuffer(m_DownSampleFBO[i]);
		if (i > 0)
		{
			api->LazyBindTexture(0, E_TextureType::Texture2D, m_DownSampleTexture[i - 1]->GetHandle());
		}
		m_pDownsampleShader->Upload("threshold"_hash, settings.bloomThreshold);
		RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();

		//blur downsampled
		api->SetShader(m_pGaussianShader.get());
		for (uint32 sample = 0; sample < static_cast<uint32>(graphicsSettings.NumBlurPasses * 2); sample++)
		{
			// #todo: needs custom ping pong buffer, buffers textures are wrong size
			bool horizontal = sample % 2 == 0;
			//output is the current framebuffer, or on the last item the framebuffer of the downsample texture
			api->BindFramebuffer(horizontal ? m_DownPingPongFBO[i] : m_DownSampleFBO[i]);
			//input is previous framebuffers texture, or on first item the result of downsampling
			api->LazyBindTexture(0, E_TextureType::Texture2D, (horizontal ? m_DownSampleTexture[i] : m_DownPingPongTexture[i])->GetHandle());
			m_pGaussianShader->Upload("horizontal"_hash, horizontal);
			RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
		}
	}
	api->SetViewport(ivec2(0), dim);
	//ping pong gaussian blur
	bool horizontal = true;
	api->SetShader(m_pGaussianShader.get());
	for (uint32 i = 0; i < static_cast<uint32>(graphicsSettings.NumBlurPasses * 2); i++)
	{
		api->BindFramebuffer(m_PingPongFBO[horizontal]);
		m_pGaussianShader->Upload("horizontal"_hash, horizontal);
		api->LazyBindTexture(0, E_TextureType::Texture2D, ((i == 0) ? m_ColorBuffers[1] : m_PingPongTexture[!horizontal])->GetHandle());
		RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
		horizontal = !horizontal;
	}
	//combine with hdr result
	T_FbLoc currentFb = graphicsSettings.UseFXAA ? m_PingPongFBO[1] : FBO;
	api->BindFramebuffer(currentFb);
	api->SetShader(m_pPostProcShader.get());
	api->LazyBindTexture(0, E_TextureType::Texture2D, m_CollectTex->GetHandle());
	api->LazyBindTexture(1, E_TextureType::Texture2D, m_PingPongTexture[0]->GetHandle());
	for (int32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		api->LazyBindTexture(2+i, E_TextureType::Texture2D, m_DownSampleTexture[i]->GetHandle());
	}
	m_pPostProcShader->Upload("exposure"_hash, settings.exposure);
	m_pPostProcShader->Upload("gamma"_hash, settings.gamma);
	m_pPostProcShader->Upload("bloomMult"_hash, settings.bloomMult);
	RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();

	//SceneRenderer::GetCurrent()->DrawOverlays(currentFb);//Make sure text and sprites get antialiased

	//FXAA
	if (graphicsSettings.UseFXAA)
	{
		api->BindFramebuffer(FBO);
		api->SetShader(m_pFXAAShader.get());
		m_pFXAAShader->Upload("uInverseScreen"_hash, 1.f / etm::vecCast<float>(dim));
		api->LazyBindTexture(0, E_TextureType::Texture2D, m_PingPongTexture[1]->GetHandle());
		RenderingSystems::Instance()->GetPrimitiveRenderer().Draw<primitives::Quad>();
	}
}

void PostProcessingRenderer::ResizeFBTextures()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	ivec2 const dim = Viewport::GetCurrentViewport()->GetDimensions();

	if(dim.x > m_CollectTex->GetResolution().x || dim.x > m_CollectTex->GetResolution().y)
	{
		//completely regenerate everything
		DeleteFramebuffers();
		api->LazyBindTexture( 0, E_TextureType::Texture2D, 0 );
		api->LazyBindTexture( 1, E_TextureType::Texture2D, 0 );
		api->LazyBindTexture( 2, E_TextureType::Texture2D, 0 );
		api->LazyBindTexture( 3, E_TextureType::Texture2D, 0 );
		api->LazyBindTexture( 4, E_TextureType::Texture2D, 0 );
		api->LazyBindTexture( 5, E_TextureType::Texture2D, 0 );
		api->LazyBindTexture( 6, E_TextureType::Texture2D, 0 );

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
		api->SetViewport(ivec2(0), etm::vecCast<int32>(etm::vecCast<float>(dim) * resMult));

		m_DownSampleTexture[i]->Resize(dim);
		m_DownPingPongTexture[i]->Resize(dim);
	}

	for(uint32 i = 0; i < 2; i++)
	{
		m_PingPongTexture[i]->Resize(dim);
	}
}