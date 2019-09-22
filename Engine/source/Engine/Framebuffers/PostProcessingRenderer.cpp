#include "stdafx.h"
#include "PostProcessingRenderer.h"

#include <glad/glad.h>

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/Shader.h>
#include <Engine/GraphicsHelper/ShadowRenderer.h>
#include <Engine/GraphicsHelper/PrimitiveRenderer.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/GraphicsHelper/GraphicsApiContext.h>
#include <Engine/Graphics/TextureData.h>


PostProcessingRenderer::PostProcessingRenderer()
{
}
PostProcessingRenderer::~PostProcessingRenderer()
{
	DeleteFramebuffers();
}

void PostProcessingRenderer::DeleteFramebuffers()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

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
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

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

	//Config::GetInstance()->GetWindow().WindowResizeEvent.AddListener( std::bind( &PostProcessingRenderer::ResizeFBTextures, this ) );
}

void PostProcessingRenderer::GenerateFramebuffers()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	TextureParameters params(false);
	params.minFilter = E_TextureFilterMode::Linear;
	params.magFilter = E_TextureFilterMode::Linear;
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;

	//Generate texture and fbo and rbo as initial postprocessing target
	api->GenFramebuffers(1, &m_CollectFBO);
	api->BindFramebuffer(m_CollectFBO);
	m_CollectTex = new TextureData(windowSettings.Width, windowSettings.Height, GL_RGB16F, GL_RGB, GL_FLOAT );
	m_CollectTex->Build();
	m_CollectTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, m_CollectTex->GetHandle(), 0);
	//Render Buffer for depth and stencil
	api->GenRenderBuffers(1, &m_CollectRBO);
	api->BindRenderbuffer(m_CollectRBO);
	api->SetRenderbufferStorage(GL_DEPTH24_STENCIL8, windowSettings.Dimensions);
	api->LinkRenderbufferToFbo(GL_DEPTH_STENCIL_ATTACHMENT, m_CollectRBO);

	//Generate textures for the hdr fbo to output into
	api->GenFramebuffers(1, &m_HDRoutFBO);
	api->BindFramebuffer(m_HDRoutFBO);
	for(GLuint i = 0; i < 2; i++)
	{
		m_ColorBuffers[i] = new TextureData(windowSettings.Width, windowSettings.Height, GL_RGB16F, GL_RGB, GL_FLOAT);
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
	for(GLuint i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		float resMult = 1.f / (float)std::pow(2, i + 1);
		ivec2 res = etm::vecCast<int32>(etm::vecCast<float>(windowSettings.Dimensions) * resMult);

		api->BindFramebuffer( m_DownSampleFBO[i] );
		m_DownSampleTexture[i] = new TextureData(res.x, res.y, GL_RGB16F, GL_RGB, GL_FLOAT );
		m_DownSampleTexture[i]->Build();
		m_DownSampleTexture[i]->SetParameters(params, true);
		api->LinkTextureToFbo2D(0, m_DownSampleTexture[i]->GetHandle(), 0);

		api->BindFramebuffer( m_DownPingPongFBO[i] );
		m_DownPingPongTexture[i] = new TextureData(res.x, res.y, GL_RGB16F, GL_RGB, GL_FLOAT );
		m_DownPingPongTexture[i]->Build();
		m_DownPingPongTexture[i]->SetParameters(params, true);
		api->LinkTextureToFbo2D(0, m_DownPingPongTexture[i]->GetHandle(), 0);
	}

	//Generate framebuffers and textures for gaussian ping pong
	api->GenFramebuffers( 2, m_PingPongFBO );
	for(GLuint i = 0; i < 2; i++)
	{
		api->BindFramebuffer( m_PingPongFBO[i] );
		m_PingPongTexture[i] = new TextureData(windowSettings.Width, windowSettings.Height, GL_RGB16F, GL_RGB, GL_FLOAT);
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
void PostProcessingRenderer::Draw(GLuint FBO, const PostProcessingSettings &settings)
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();
	Config::Settings::Graphics const& graphicsSettings = Config::GetInstance()->GetGraphics();

	api->SetDepthEnabled(false);
	//get glow
	api->BindFramebuffer(m_HDRoutFBO);
	api->SetShader(m_pDownsampleShader.get());
	api->LazyBindTexture(0, E_TextureType::Texture2D, m_CollectTex->GetHandle());
	m_pDownsampleShader->Upload("threshold"_hash, settings.bloomThreshold);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	//downsample glow
	for (GLuint i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		if (i > 0) api->SetShader(m_pDownsampleShader.get());
		float resMult = 1.f / (float)std::pow(2, i + 1);
		api->SetViewport(ivec2(0), etm::vecCast<int32>(etm::vecCast<float>(windowSettings.Dimensions) * resMult));
		api->BindFramebuffer(m_DownSampleFBO[i]);
		if (i > 0)
		{
			api->LazyBindTexture(0, E_TextureType::Texture2D, m_DownSampleTexture[i - 1]->GetHandle());
		}
		m_pDownsampleShader->Upload("threshold"_hash, settings.bloomThreshold);
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

		//blur downsampled
		//api->SetViewport(ivec2(0), windowSettings.Dimensions);
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
			PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		}
	}
	api->SetViewport(ivec2(0), windowSettings.Dimensions);
	//ping pong gaussian blur
	bool horizontal = true;
	api->SetShader(m_pGaussianShader.get());
	for (GLuint i = 0; i < (GLuint)graphicsSettings.NumBlurPasses * 2; i++)
	{
		api->BindFramebuffer(m_PingPongFBO[horizontal]);
		m_pGaussianShader->Upload("horizontal"_hash, horizontal);
		api->LazyBindTexture(0, E_TextureType::Texture2D, ((i == 0) ? m_ColorBuffers[1] : m_PingPongTexture[!horizontal])->GetHandle());
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		horizontal = !horizontal;
	}
	//combine with hdr result
	if (graphicsSettings.UseFXAA)
	{
		// use the second pingpong fbo to store FXAA
		api->BindFramebuffer(m_PingPongFBO[1]);
	}
	else
	{
		api->BindFramebuffer(FBO);
	}
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
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	RenderPipeline::GetInstance()->DrawOverlays();//Make sure text and sprites get antialiased

	//FXAA
	if (graphicsSettings.UseFXAA)
	{
		api->BindFramebuffer(FBO);
		api->SetShader(m_pFXAAShader.get());
		m_pFXAAShader->Upload("uInverseScreen"_hash, 1.f / etm::vecCast<float>(windowSettings.Dimensions));
		api->LazyBindTexture(0, E_TextureType::Texture2D, m_PingPongTexture[1]->GetHandle());
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	}
}

void PostProcessingRenderer::ResizeFBTextures()
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	bool upsize = windowSettings.Width > m_CollectTex->GetResolution().x || windowSettings.Height > m_CollectTex->GetResolution().y;
	if(upsize)
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

	m_CollectTex->Resize(windowSettings.Dimensions );
	for(uint32 i = 0; i < 2; i++)
	{
		m_ColorBuffers[i]->Resize(windowSettings.Dimensions );
	}
	for(GLuint i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		//api->BindFramebuffer( m_DownSampleFBO[i] );
		float resMult = 1.f / (float)std::pow( 2, i + 1 );
		api->SetViewport(ivec2(0), etm::vecCast<int32>(etm::vecCast<float>(windowSettings.Dimensions) * resMult));

		m_DownSampleTexture[i]->Resize(windowSettings.Dimensions);
		m_DownPingPongTexture[i]->Resize(windowSettings.Dimensions);
	}
	for(GLuint i = 0; i < 2; i++)
	{
		m_PingPongTexture[i]->Resize(windowSettings.Dimensions );
	}
}