#include "stdafx.hpp"
#include "PostProcessingRenderer.hpp"

#include "../Graphics/ShaderData.hpp"
#include "../GraphicsHelper/ShadowRenderer.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"

PostProcessingRenderer::PostProcessingRenderer()
{
}
PostProcessingRenderer::~PostProcessingRenderer()
{
	glDeleteRenderbuffers(1, &m_CollectRBO);
	glDeleteTextures(1, &m_CollectTex);
	glDeleteFramebuffers(1, &m_CollectFBO);

	glDeleteTextures(2, m_ColorBuffers);
	glDeleteFramebuffers(1, &m_HDRoutFBO);

	glDeleteTextures(2, m_PingPongTexture);
	glDeleteFramebuffers(2, m_PingPongFBO);

	glDeleteTextures(NUM_BLOOM_DOWNSAMPLES, m_DownSampleTexture);
	glDeleteFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownSampleFBO);
	glDeleteTextures(NUM_BLOOM_DOWNSAMPLES, m_DownPingPongTexture);
	glDeleteFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownPingPongFBO);
}

void PostProcessingRenderer::Initialize()
{
	//Load and compile Shaders
	m_pDownsampleShader = ContentManager::Load<ShaderData>("Shaders/PostDownsample.glsl");
	m_pGaussianShader = ContentManager::Load<ShaderData>("Shaders/PostGaussian.glsl");
	m_pPostProcShader = ContentManager::Load<ShaderData>("Shaders/PostProcessing.glsl");

	//Access shader variables
	glUseProgram(m_pDownsampleShader->GetProgram());
	glUniform1i(glGetUniformLocation(m_pDownsampleShader->GetProgram(), "texColor"), 0);
	m_uThreshold = glGetUniformLocation(m_pDownsampleShader->GetProgram(), "threshold");

	glUseProgram(m_pGaussianShader->GetProgram());
	glUniform1i(glGetUniformLocation(m_pGaussianShader->GetProgram(), "image"), 0);
	m_uHorizontal = glGetUniformLocation(m_pGaussianShader->GetProgram(), "horizontal");

	glUseProgram(m_pPostProcShader->GetProgram());
	glUniform1i(glGetUniformLocation(m_pPostProcShader->GetProgram(), "texColor"), 0);
	for (int i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		glUniform1i(glGetUniformLocation(m_pPostProcShader->GetProgram(), 
			(std::string("texBloom") + std::to_string(i)).c_str()), i+1);
	}
	m_uExposure = glGetUniformLocation(m_pPostProcShader->GetProgram(), "exposure");
	m_uGamma = glGetUniformLocation(m_pPostProcShader->GetProgram(), "gamma");
	m_uBloomMult = glGetUniformLocation(m_pPostProcShader->GetProgram(), "bloomMult");

	int width = SETTINGS->Window.Width, height = SETTINGS->Window.Height;

	//Generate texture and fbo and rbo as initial postprocessing target
	glGenFramebuffers(1, &m_CollectFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_CollectFBO);
	glGenTextures(1, &m_CollectTex);
	glBindTexture(GL_TEXTURE_2D, m_CollectTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_CollectTex, 0);
	//Render Buffer for depth and stencil
	glGenRenderbuffers(1, &m_CollectRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_CollectRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SETTINGS->Window.Width, SETTINGS->Window.Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_CollectRBO);

	//Generate textures for the hdr fbo to output into
	glGenFramebuffers(1, &m_HDRoutFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_HDRoutFBO);
	glGenTextures(2, m_ColorBuffers);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, m_ColorBuffers[i]);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_ColorBuffers[i], 0 );
	}
	//mrt
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	//Generate framebuffers for downsampling
	glGenFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownSampleFBO);
	glGenTextures(NUM_BLOOM_DOWNSAMPLES, m_DownSampleTexture);
	glGenFramebuffers(NUM_BLOOM_DOWNSAMPLES, m_DownPingPongFBO);
	glGenTextures(NUM_BLOOM_DOWNSAMPLES, m_DownPingPongTexture);
	for (GLuint i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++)
	{
		float resMult = 1.f / (float)std::pow(2, i + 1);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DownSampleFBO[i]);
		glBindTexture(GL_TEXTURE_2D, m_DownSampleTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, (GLsizei)(width*resMult), (GLsizei)(height*resMult), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_DownSampleTexture[i], 0);

		glBindFramebuffer(GL_FRAMEBUFFER, m_DownPingPongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, m_DownPingPongTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, (GLsizei)(width*resMult), (GLsizei)(height*resMult), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_DownPingPongTexture[i], 0);
	}

	//Generate framebuffers and textures for gaussian ping pong
	glGenFramebuffers(2, m_PingPongFBO);
	glGenTextures(2, m_PingPongTexture);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, m_PingPongTexture[i]);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PingPongTexture[i], 0 );
	}

	if (!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE))
		LOGGER::Log("Framebuffer>Initialize() FAILED!", LogLevel::Error);
}
void PostProcessingRenderer::EnableInput()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_CollectFBO);
}
void PostProcessingRenderer::Draw(GLuint FBO)
{
	glDisable(GL_DEPTH_TEST);
	//get glow
	glBindFramebuffer(GL_FRAMEBUFFER, m_HDRoutFBO);
	glUseProgram(m_pDownsampleShader->GetProgram());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_CollectTex);
	glUniform1f(m_uThreshold, m_Threshold);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	//downsample glow
	int width = SETTINGS->Window.Width, height = SETTINGS->Window.Height;
	for (GLuint i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		if(i>0) glUseProgram(m_pDownsampleShader->GetProgram());
		float resMult = 1.f / (float)std::pow(2, i + 1);
		glViewport(0, 0, (GLsizei)(width*resMult), (GLsizei)(height*resMult));
		glBindFramebuffer(GL_FRAMEBUFFER, m_DownSampleFBO[i]);
		if(i>0)glBindTexture(GL_TEXTURE_2D, m_DownSampleTexture[i-1]);
		glUniform1f(m_uThreshold, m_Threshold);
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

		//blur downsampled
		//glViewport(0, 0, width, height);
		glUseProgram(m_pGaussianShader->GetProgram());
		for (GLuint j = 0; j < (GLuint)GRAPHICS.NumBlurPasses * 2; j++)
		{
			//TODO needs custom ping pong buffer, buffers textures are wrong size
			GLboolean horizontal = !(GLboolean)(j % 2);
			//output is the current framebuffer, or on the last item the framebuffer of the downsample texture
			glBindFramebuffer(GL_FRAMEBUFFER, horizontal ? m_DownPingPongFBO[i] : m_DownSampleFBO[i]);
			//input is previous framebuffers texture, or on first item the result of downsampling
			glBindTexture(GL_TEXTURE_2D, horizontal ? m_DownSampleTexture[i] : m_DownPingPongTexture[i]);
			glUniform1i(m_uHorizontal, horizontal);
			PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		}
	}
	glViewport(0, 0, width, height);
	//ping pong gaussian blur
	GLboolean horizontal = true;
	glUseProgram(m_pGaussianShader->GetProgram());
	for (GLuint i = 0; i < (GLuint)GRAPHICS.NumBlurPasses * 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFBO[horizontal]);
		glUniform1i(m_uHorizontal, horizontal);
		glBindTexture( GL_TEXTURE_2D, (i==0) ? m_ColorBuffers[1] : m_PingPongTexture[!horizontal] );
		PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
		horizontal = !horizontal;
	}
	//combine with hdr result
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glUseProgram(m_pPostProcShader->GetProgram());
	glBindTexture(GL_TEXTURE_2D, m_CollectTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_PingPongTexture[0]);
	for (int i = 0; i < NUM_BLOOM_DOWNSAMPLES; ++i)
	{
		glActiveTexture(GL_TEXTURE2 + i);
		glBindTexture(GL_TEXTURE_2D, m_DownSampleTexture[i]);
	}
	glUniform1f(m_uExposure, m_Exposure);
	glUniform1f(m_uGamma, m_Gamma);
	glUniform1f(m_uBloomMult, m_BloomMult);
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}