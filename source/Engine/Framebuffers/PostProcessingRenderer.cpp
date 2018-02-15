#include "stdafx.hpp"
#include "PostProcessingRenderer.hpp"

#include "../Graphics/ShaderData.hpp"
#include "../GraphicsHelper/ShadowRenderer.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"
#include "../GraphicsHelper/RenderPipeline.hpp"
#include "../GraphicsHelper/RenderState.hpp"
#include "TextureData.hpp"

PostProcessingRenderer::PostProcessingRenderer()
{
}
PostProcessingRenderer::~PostProcessingRenderer()
{
	DeleteFramebuffers();
}

void PostProcessingRenderer::DeleteFramebuffers()
{
	glDeleteRenderbuffers(1, &m_CollectRBO);
	delete m_CollectTex; m_CollectTex = nullptr;
	glDeleteFramebuffers(1, &m_CollectFBO);

	delete m_ColorBuffers[0]; m_ColorBuffers[0] = nullptr;
	delete m_ColorBuffers[1]; m_ColorBuffers[1] = nullptr;
	glDeleteFramebuffers(1, &m_HDRoutFBO);

	delete m_PingPongTexture[0]; m_PingPongTexture[0] = nullptr;
	delete m_PingPongTexture[1]; m_PingPongTexture[1] = nullptr;
	glDeleteFramebuffers(2, &(m_PingPongFBO[0]));

	for(uint32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		delete m_DownSampleTexture[i]; m_DownSampleTexture[i] = nullptr;
		delete m_DownPingPongTexture[i]; m_DownPingPongTexture[i] = nullptr;
	}
	glDeleteFramebuffers(NUM_BLOOM_DOWNSAMPLES, &(m_DownSampleFBO[0]));
	glDeleteFramebuffers(NUM_BLOOM_DOWNSAMPLES, &(m_DownPingPongFBO[0]));
}

void PostProcessingRenderer::Initialize()
{
	//Load and compile Shaders
	m_pDownsampleShader = ContentManager::Load<ShaderData>("Shaders/PostDownsample.glsl");
	m_pGaussianShader = ContentManager::Load<ShaderData>("Shaders/PostGaussian.glsl");
	m_pPostProcShader = ContentManager::Load<ShaderData>("Shaders/PostProcessing.glsl");
	m_pFXAAShader = ContentManager::Load<ShaderData>("Shaders/PostFXAA.glsl");

	//Access shader variables
	STATE->SetShader(m_pDownsampleShader);
	glUniform1i(glGetUniformLocation(m_pDownsampleShader->GetProgram(), "texColor"), 0);
	m_uThreshold = glGetUniformLocation(m_pDownsampleShader->GetProgram(), "threshold");

	STATE->SetShader(m_pGaussianShader);
	glUniform1i(glGetUniformLocation(m_pGaussianShader->GetProgram(), "image"), 0);
	m_uHorizontal = glGetUniformLocation(m_pGaussianShader->GetProgram(), "horizontal");

	STATE->SetShader(m_pPostProcShader);
	glUniform1i(glGetUniformLocation(m_pPostProcShader->GetProgram(), "texColor"), 0);
	for (int32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		glUniform1i(glGetUniformLocation(m_pPostProcShader->GetProgram(), 
			(std::string("texBloom") + std::to_string(i)).c_str()), i+1);
	}
	m_uExposure = glGetUniformLocation(m_pPostProcShader->GetProgram(), "exposure");
	m_uGamma = glGetUniformLocation(m_pPostProcShader->GetProgram(), "gamma");
	m_uBloomMult = glGetUniformLocation(m_pPostProcShader->GetProgram(), "bloomMult");

	STATE->SetShader(m_pFXAAShader);
	glUniform1i(glGetUniformLocation(m_pFXAAShader->GetProgram(), "texColor"), 0);
	m_uInverseScreen = glGetUniformLocation(m_pFXAAShader->GetProgram(), "uInverseScreen");

	GenerateFramebuffers();

	//WINDOW.WindowResizeEvent.AddListener( std::bind( &PostProcessingRenderer::ResizeFBTextures, this ) );
}

void PostProcessingRenderer::GenerateFramebuffers()
{
	int32 width = WINDOW.Width, height = WINDOW.Height;

	TextureParameters params = TextureParameters();
	params.minFilter = GL_LINEAR;
	params.magFilter = GL_LINEAR;
	params.wrapS = GL_CLAMP_TO_EDGE;
	params.wrapT = GL_CLAMP_TO_EDGE;

	//Generate texture and fbo and rbo as initial postprocessing target
	glGenFramebuffers( 1, &m_CollectFBO );
	STATE->BindFramebuffer( m_CollectFBO );
	m_CollectTex = new TextureData( width, height, GL_RGB16F, GL_RGB, GL_FLOAT );
	m_CollectTex->Build();
	m_CollectTex->SetParameters( params );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_CollectTex->GetHandle(), 0 );
	//Render Buffer for depth and stencil
	glGenRenderbuffers( 1, &m_CollectRBO );
	glBindRenderbuffer( GL_RENDERBUFFER, m_CollectRBO );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_CollectRBO );

	//Generate textures for the hdr fbo to output into
	glGenFramebuffers( 1, &m_HDRoutFBO );
	STATE->BindFramebuffer( m_HDRoutFBO );
	for(GLuint i = 0; i < 2; i++)
	{
		m_ColorBuffers[i] = new TextureData( width, height, GL_RGB16F, GL_RGB, GL_FLOAT );
		m_ColorBuffers[i]->Build();
		m_ColorBuffers[i]->SetParameters( params );
		// attach texture to framebuffer
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_ColorBuffers[i]->GetHandle(), 0 );
	}
	//mrt
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers( 2, attachments );

	//Generate framebuffers for downsampling
	glGenFramebuffers( NUM_BLOOM_DOWNSAMPLES, m_DownSampleFBO );
	glGenFramebuffers( NUM_BLOOM_DOWNSAMPLES, m_DownPingPongFBO );
	for(GLuint i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		float resMult = 1.f / (float)std::pow( 2, i + 1 );
		STATE->BindFramebuffer( m_DownSampleFBO[i] );
		m_DownSampleTexture[i] = new TextureData( (GLsizei)(width*resMult), (GLsizei)(height*resMult), GL_RGB16F, GL_RGB, GL_FLOAT );
		m_DownSampleTexture[i]->Build();
		m_DownSampleTexture[i]->SetParameters( params );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_DownSampleTexture[i]->GetHandle(), 0 );

		STATE->BindFramebuffer( m_DownPingPongFBO[i] );
		m_DownPingPongTexture[i] = new TextureData( (GLsizei)(width*resMult), (GLsizei)(height*resMult), GL_RGB16F, GL_RGB, GL_FLOAT );
		m_DownPingPongTexture[i]->Build();
		m_DownPingPongTexture[i]->SetParameters( params );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_DownPingPongTexture[i]->GetHandle(), 0 );
	}

	//Generate framebuffers and textures for gaussian ping pong
	glGenFramebuffers( 2, m_PingPongFBO );
	for(GLuint i = 0; i < 2; i++)
	{
		STATE->BindFramebuffer( m_PingPongFBO[i] );
		m_PingPongTexture[i] = new TextureData( width, height, GL_RGB16F, GL_RGB, GL_FLOAT );
		m_PingPongTexture[i]->Build();
		m_PingPongTexture[i]->SetParameters( params );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PingPongTexture[i]->GetHandle(), 0 );
	}

	if(!(glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE))
		LOG( "Framebuffer>Initialize() FAILED!", LogLevel::Error );
}

void PostProcessingRenderer::EnableInput()
{
	STATE->BindFramebuffer(m_CollectFBO);
}
void PostProcessingRenderer::Draw(GLuint FBO, const PostProcessingSettings &settings)
{
	STATE->SetDepthEnabled(false);
	//get glow
	STATE->BindFramebuffer(m_HDRoutFBO);
	STATE->SetShader(m_pDownsampleShader);
	STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_CollectTex->GetHandle());
	glUniform1f(m_uThreshold, settings.bloomThreshold);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	//downsample glow
	int32 width = SETTINGS->Window.Width, height = SETTINGS->Window.Height;
	for (GLuint i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		if (i > 0) STATE->SetShader(m_pDownsampleShader);
		float resMult = 1.f / (float)std::pow(2, i + 1);
		STATE->SetViewport(ivec2(0), ivec2((int32)(width*resMult), (int32)(height*resMult)));
		STATE->BindFramebuffer(m_DownSampleFBO[i]);
		if(i>0) STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_DownSampleTexture[i - 1]->GetHandle());
		glUniform1f(m_uThreshold, settings.bloomThreshold);
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

		//blur downsampled
		//STATE->SetViewport(ivec2(0), ivec2(width, height));
		STATE->SetShader(m_pGaussianShader);
	#ifdef EDITOR
		for (uint32 sample = 0; sample < 10; sample++)
	#else
		for (uint32 sample = 0; sample < static_cast<uint32>(GRAPHICS.NumBlurPasses * 2); sample++)
	#endif
		{
			// #todo: needs custom ping pong buffer, buffers textures are wrong size
			bool horizontal = sample % 2 == 0;
			//output is the current framebuffer, or on the last item the framebuffer of the downsample texture
			STATE->BindFramebuffer(horizontal ? m_DownPingPongFBO[i] : m_DownSampleFBO[i]);
			//input is previous framebuffers texture, or on first item the result of downsampling
			STATE->LazyBindTexture(0, GL_TEXTURE_2D, (horizontal ? m_DownSampleTexture[i] : m_DownPingPongTexture[i])->GetHandle());
			glUniform1i(m_uHorizontal, horizontal);
			PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		}
	}
	STATE->SetViewport(ivec2(0), ivec2(width, height));
	//ping pong gaussian blur
	GLboolean horizontal = true;
	STATE->SetShader(m_pGaussianShader);
#ifdef EDITOR
	for (GLuint i = 0; i < 10; i++)
#else
	for (GLuint i = 0; i < (GLuint)GRAPHICS.NumBlurPasses * 2; i++)
#endif
	{
		STATE->BindFramebuffer(m_PingPongFBO[horizontal]);
		glUniform1i(m_uHorizontal, horizontal);
		STATE->LazyBindTexture(0, GL_TEXTURE_2D, ((i == 0) ? m_ColorBuffers[1] : m_PingPongTexture[!horizontal])->GetHandle());
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		horizontal = !horizontal;
	}
	//combine with hdr result
	if (GRAPHICS.UseFXAA)
	{
		// use the second pingpong fbo to store FXAA
		STATE->BindFramebuffer(m_PingPongFBO[1]);
	}
	else
	{
		STATE->BindFramebuffer(FBO);
	}
	STATE->SetShader(m_pPostProcShader);
	STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_CollectTex->GetHandle());
	STATE->LazyBindTexture(1, GL_TEXTURE_2D, m_PingPongTexture[0]->GetHandle());
	for (int32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		STATE->LazyBindTexture(2+i, GL_TEXTURE_2D, m_DownSampleTexture[i]->GetHandle());
	}
	glUniform1f(m_uExposure, settings.exposure);
	glUniform1f(m_uGamma, settings.gamma);
	glUniform1f(m_uBloomMult, settings.bloomMult);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	//FXAA
	if (GRAPHICS.UseFXAA)
	{
		STATE->BindFramebuffer(FBO);
		STATE->SetShader(m_pFXAAShader);
		glUniform2f(m_uInverseScreen, 1.f/(float)WINDOW.Width, 1.f/(float)WINDOW.Height);
		STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_PingPongTexture[1]->GetHandle());
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	}
}

void PostProcessingRenderer::ResizeFBTextures()
{
	int32 width = WINDOW.Width, height = WINDOW.Height;

	bool upsize = WINDOW.Width > m_CollectTex->GetResolution().x || WINDOW.Height > m_CollectTex->GetResolution().y;
	if(upsize)
	{
		//completely regenerate everything
		DeleteFramebuffers();
		STATE->LazyBindTexture( 0, GL_TEXTURE_2D, 0 );
		STATE->LazyBindTexture( 1, GL_TEXTURE_2D, 0 );
		STATE->LazyBindTexture( 2, GL_TEXTURE_2D, 0 );
		STATE->LazyBindTexture( 3, GL_TEXTURE_2D, 0 );
		STATE->LazyBindTexture( 4, GL_TEXTURE_2D, 0 );
		STATE->LazyBindTexture( 5, GL_TEXTURE_2D, 0 );
		STATE->LazyBindTexture( 6, GL_TEXTURE_2D, 0 );
		//Access shader variables
		STATE->SetShader( m_pDownsampleShader );
		glUniform1i( glGetUniformLocation( m_pDownsampleShader->GetProgram(), "texColor" ), 0 );
		m_uThreshold = glGetUniformLocation( m_pDownsampleShader->GetProgram(), "threshold" );

		STATE->SetShader( m_pGaussianShader );
		glUniform1i( glGetUniformLocation( m_pGaussianShader->GetProgram(), "image" ), 0 );
		m_uHorizontal = glGetUniformLocation( m_pGaussianShader->GetProgram(), "horizontal" );

		STATE->SetShader( m_pPostProcShader );
		glUniform1i( glGetUniformLocation( m_pPostProcShader->GetProgram(), "texColor" ), 0 );
		for(int32 i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
		{
			glUniform1i( glGetUniformLocation( m_pPostProcShader->GetProgram(),
				(std::string( "texBloom" ) + std::to_string( i )).c_str() ), i + 1 );
		}
		m_uExposure = glGetUniformLocation( m_pPostProcShader->GetProgram(), "exposure" );
		m_uGamma = glGetUniformLocation( m_pPostProcShader->GetProgram(), "gamma" );
		m_uBloomMult = glGetUniformLocation( m_pPostProcShader->GetProgram(), "bloomMult" );
		GenerateFramebuffers();
		return;
	}

	m_CollectTex->Resize( WINDOW.Dimensions );
	for(uint32 i = 0; i < 2; i++)
	{
		m_ColorBuffers[i]->Resize( WINDOW.Dimensions );
	}
	for(GLuint i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		//STATE->BindFramebuffer( m_DownSampleFBO[i] );
		float resMult = 1.f / (float)std::pow( 2, i + 1 );

		ivec2 dimensions = ivec2( (int32)((float)width * resMult, (float)height * resMult) );

		m_DownSampleTexture[i]->Resize( dimensions );
		m_DownPingPongTexture[i]->Resize( dimensions );
	}
	for(GLuint i = 0; i < 2; i++)
	{
		m_PingPongTexture[i]->Resize( WINDOW.Dimensions );
	}
}