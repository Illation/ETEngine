#include "stdafx.hpp"

#include "FrameBuffer.hpp"

#include "ShaderData.hpp"
#include "TextureData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"

FrameBuffer::FrameBuffer(string shaderFile, GLenum format, uint32 numTargets)
	:m_ShaderFile(shaderFile),
	m_Format(format),
	m_NumTargets(numTargets)
{
}
FrameBuffer::~FrameBuffer()
{
	glDeleteRenderbuffers(1, &m_RboDepthStencil);
	for (size_t i = 0; i < m_pTextureVec.size(); i++)
	{
		SafeDelete(m_pTextureVec[i]);
	}
	glDeleteFramebuffers(1, &m_GlFrameBuffer);
}

void FrameBuffer::Initialize()
{
	//Load and compile Shaders
	m_pShader = ContentManager::Load<ShaderData>(m_ShaderFile);

	//GetAccessTo shader attributes
	STATE->SetShader(m_pShader);
	AccessShaderAttributes();

	//FrameBuffer
	glGenFramebuffers(1, &m_GlFrameBuffer);
	STATE->BindFramebuffer(m_GlFrameBuffer);
	//Textures
	int32 width = SETTINGS->Window.Width, height = SETTINGS->Window.Height;
	vector<GLuint> attachments;
	attachments.reserve(m_NumTargets);
	m_pTextureVec.reserve(m_NumTargets);
	//Depth buffer
	if (m_CaptureDepth)
	{
		GLuint depthMap;
		glGenTextures(1, &depthMap);
		STATE->BindTexture(GL_TEXTURE_2D, depthMap);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, /*float*/GL_UNSIGNED_INT_24_8, NULL);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		m_pTextureVec.push_back(new TextureData(depthMap, width, height));
		//texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	//Color buffers
	for (size_t i = 0; i < m_NumTargets; i++)
	{
		GLuint texHandle;
		glGenTextures(1, &texHandle);
		STATE->BindTexture(GL_TEXTURE_2D, texHandle);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, m_Format, NULL );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texHandle, 0 );
		m_pTextureVec.push_back(new TextureData(texHandle, width, height));
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		//texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	//Render Buffer for depth and stencil
	if (!m_CaptureDepth)
	{
		glGenRenderbuffers(1, &m_RboDepthStencil);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RboDepthStencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RboDepthStencil);
	}

	glDrawBuffers(m_NumTargets, attachments.data());

	if (!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE))
		LOGGER::Log("Framebuffer>Initialize() FAILED!", LogLevel::Error);
}

void FrameBuffer::AccessShaderAttributes()
{
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "texFramebuffer"), 0);
}

void FrameBuffer::Enable(bool active)
{
	if (active) STATE->BindFramebuffer(m_GlFrameBuffer);
	else STATE->BindFramebuffer(0);
}

void FrameBuffer::Draw()
{
	STATE->SetDepthEnabled(false);
	STATE->SetShader(m_pShader);
	for (size_t i = 0; i < m_pTextureVec.size(); i++)
	{
		STATE->LazyBindTexture(i, GL_TEXTURE_2D, m_pTextureVec[i]->GetHandle());
	}

	UploadDerivedVariables();

	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}