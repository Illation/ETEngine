#include "stdafx.hpp"

#include "FrameBuffer.hpp"

#include "ShaderData.hpp"
#include "TextureData.hpp"
#include "../GraphicsHelper/PrimitiveRenderer.hpp"

FrameBuffer::FrameBuffer(std::string shaderFile, GLenum format, uint32 numTargets)
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

	GenerateFramebufferTextures();

	if (!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE))
		LOG("Framebuffer>Initialize() FAILED!", LogLevel::Error);

	WINDOW.WindowResizeEvent.AddListener( std::bind( &FrameBuffer::ResizeFramebufferTextures, this ) );
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
	for (uint32 i = 0; i < (uint32)m_pTextureVec.size(); i++)
	{
		STATE->LazyBindTexture(i, GL_TEXTURE_2D, m_pTextureVec[i]->GetHandle());
	}

	UploadDerivedVariables();

	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}


void FrameBuffer::GenerateFramebufferTextures()
{
	STATE->BindFramebuffer(m_GlFrameBuffer);
	//Textures
	int32 width = WINDOW.Width, height = WINDOW.Height;
	std::vector<GLuint> attachments;
	attachments.reserve(m_NumTargets);
	m_pTextureVec.reserve(m_NumTargets);

	TextureParameters params = TextureParameters();
	params.minFilter = GL_NEAREST;
	params.magFilter = GL_NEAREST;
	params.wrapS = GL_CLAMP_TO_EDGE;
	params.wrapT = GL_CLAMP_TO_EDGE;
	//Depth buffer
	if (m_CaptureDepth)
	{
		TextureData* depthMap = new TextureData( width, height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT );
		depthMap->Build();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap->GetHandle(), 0);
		depthMap->SetParameters( params );
		m_pTextureVec.push_back( depthMap );
	}

	//Color buffers
	for (uint32 i = 0; i < m_NumTargets; i++)
	{
		TextureData* colorBuffer = new TextureData( width, height, GL_RGBA16F, GL_RGBA, m_Format );
		colorBuffer->Build();
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer->GetHandle(), 0 );
		colorBuffer->SetParameters( params );
		m_pTextureVec.push_back( colorBuffer );
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
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
}

void FrameBuffer::ResizeFramebufferTextures()
{
	int32 width = WINDOW.Width, height = WINDOW.Height;
	assert( m_pTextureVec.size() > 0 );
	bool upscale = WINDOW.Width > m_pTextureVec[0]->GetResolution().x || WINDOW.Height > m_pTextureVec[0]->GetResolution().y;
	if(upscale)
	{
		glDeleteRenderbuffers( 1, &m_RboDepthStencil );
		for(uint32 i = 0; i < m_pTextureVec.size(); i++)
		{
			SafeDelete( m_pTextureVec[i] );
		}
		m_pTextureVec.clear();
		glDeleteFramebuffers( 1, &m_GlFrameBuffer );
		glGenFramebuffers( 1, &m_GlFrameBuffer );
		GenerateFramebufferTextures();
		return;
	}

	uint32 offset = 0;
	if(m_CaptureDepth)
	{
		m_pTextureVec[0]->Resize( WINDOW.Dimensions );
		++offset;
	}
	else 
	{
		//completely regenerate the renderbuffer object
		STATE->BindFramebuffer(m_GlFrameBuffer);
		glDeleteRenderbuffers( 1, &m_RboDepthStencil );
		glGenRenderbuffers( 1, &m_RboDepthStencil );
		glBindRenderbuffer( GL_RENDERBUFFER, m_RboDepthStencil );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RboDepthStencil );
	}
	assert( m_pTextureVec.size() >= offset + m_NumTargets );
	for(uint32 i = offset; i < offset + m_NumTargets; ++i)
	{
		m_pTextureVec[i]->Resize( WINDOW.Dimensions );
	}
}
