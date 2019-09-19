#include "stdafx.h"
#include "FrameBuffer.h"

#include <glad/glad.h>

#include "Shader.h"
#include "TextureData.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>

#include <Engine/GraphicsHelper/PrimitiveRenderer.h>


FrameBuffer::FrameBuffer(std::string shaderFile, GLenum format, uint32 numTargets)
	:m_ShaderFile(shaderFile),
	m_Format(format),
	m_NumTargets(numTargets)
{
}
FrameBuffer::~FrameBuffer()
{
	STATE->DeleteRenderBuffers(1, &m_RboDepthStencil);
	for (size_t i = 0; i < m_pTextureVec.size(); i++)
	{
		SafeDelete(m_pTextureVec[i]);
	}
	STATE->DeleteFramebuffers(1, &m_GlFrameBuffer);
}

void FrameBuffer::Initialize()
{
	//Load and compile Shaders
	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>(GetHash(FileUtil::ExtractName(m_ShaderFile)));

	//GetAccessTo shader attributes
	STATE->SetShader(m_pShader.get());
	AccessShaderAttributes();

	//FrameBuffer
	STATE->GenFramebuffers(1, &m_GlFrameBuffer);

	GenerateFramebufferTextures();

	if (!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE))
		LOG("Framebuffer>Initialize() FAILED!", LogLevel::Error);

	Config::GetInstance()->GetWindow().WindowResizeEvent.AddListener(std::bind( &FrameBuffer::ResizeFramebufferTextures, this));
}

void FrameBuffer::AccessShaderAttributes()
{
	m_pShader->Upload("texFramebuffer"_hash, 0);
}

void FrameBuffer::Enable(bool active)
{
	if (active) STATE->BindFramebuffer(m_GlFrameBuffer);
	else STATE->BindFramebuffer(0);
}

void FrameBuffer::Draw()
{
	STATE->SetDepthEnabled(false);
	STATE->SetShader(m_pShader.get());
	for (uint32 i = 0; i < (uint32)m_pTextureVec.size(); i++)
	{
		STATE->LazyBindTexture(i, GL_TEXTURE_2D, m_pTextureVec[i]->GetHandle());
	}

	UploadDerivedVariables();

	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}


void FrameBuffer::GenerateFramebufferTextures()
{
	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	STATE->BindFramebuffer(m_GlFrameBuffer);

	//Textures
	std::vector<GLuint> attachments;
	attachments.reserve(m_NumTargets);
	m_pTextureVec.reserve(m_NumTargets);

	TextureParameters params(false);
	params.wrapS = E_TextureWrapMode::ClampToEdge;
	params.wrapT = E_TextureWrapMode::ClampToEdge;
	//Depth buffer
	if (m_CaptureDepth)
	{
		TextureData* depthMap = new TextureData(windowSettings.Width, windowSettings.Height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT );
		depthMap->Build();
		STATE->LinkTextureToDepth(GL_TEXTURE_2D, depthMap->GetHandle());
		depthMap->SetParameters(params);
		m_pTextureVec.emplace_back(depthMap);
	}

	//Color buffers
	for (uint32 i = 0; i < m_NumTargets; i++)
	{
		TextureData* colorBuffer = new TextureData(windowSettings.Width, windowSettings.Height, GL_RGBA16F, GL_RGBA, m_Format );
		colorBuffer->Build();
		STATE->LinkTextureToFbo2D(i, GL_TEXTURE_2D, colorBuffer->GetHandle(), 0);
		colorBuffer->SetParameters(params, true);
		m_pTextureVec.emplace_back(colorBuffer);
		attachments.emplace_back(GL_COLOR_ATTACHMENT0 + i);
	}

	//Render Buffer for depth and stencil
	if (!m_CaptureDepth)
	{
		STATE->GenRenderBuffers(1, &m_RboDepthStencil);
		STATE->BindRenderbuffer(m_RboDepthStencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowSettings.Width, windowSettings.Height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RboDepthStencil);
	}

	glDrawBuffers(m_NumTargets, attachments.data());
}

void FrameBuffer::ResizeFramebufferTextures()
{
	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	ET_ASSERT(m_pTextureVec.size() > 0);

	if(windowSettings.Width > m_pTextureVec[0]->GetResolution().x || windowSettings.Height > m_pTextureVec[0]->GetResolution().y)
	{
		STATE->DeleteRenderBuffers( 1, &m_RboDepthStencil );
		for(uint32 i = 0; i < m_pTextureVec.size(); i++)
		{
			SafeDelete( m_pTextureVec[i] );
		}
		m_pTextureVec.clear();
		STATE->DeleteFramebuffers( 1, &m_GlFrameBuffer );
		STATE->GenFramebuffers( 1, &m_GlFrameBuffer );
		GenerateFramebufferTextures();
		return;
	}

	uint32 offset = 0;
	if(m_CaptureDepth)
	{
		m_pTextureVec[0]->Resize(windowSettings.Dimensions);
		++offset;
	}
	else 
	{
		//completely regenerate the renderbuffer object
		STATE->BindFramebuffer(m_GlFrameBuffer);
		STATE->DeleteRenderBuffers( 1, &m_RboDepthStencil );
		STATE->GenRenderBuffers(1, &m_RboDepthStencil);
		STATE->BindRenderbuffer(m_RboDepthStencil);
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowSettings.Width, windowSettings.Height);
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RboDepthStencil);
	}
	assert( m_pTextureVec.size() >= offset + m_NumTargets );
	for(uint32 i = offset; i < offset + m_NumTargets; ++i)
	{
		m_pTextureVec[i]->Resize(windowSettings.Dimensions);
	}
}
