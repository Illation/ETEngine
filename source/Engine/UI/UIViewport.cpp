#include "stdafx.hpp"
#include "UIViewport.h"
#include "PrimitiveRenderer.hpp"
#include "TextureData.hpp"
#include "ShaderData.hpp"

UIViewport::UIViewport():UIFixedContainer()
{
	m_Renderer = new UIViewportRenderer();
}

UIViewport::~UIViewport()
{
	delete m_Renderer;
}


void UIViewport::Initialize()
{
	m_Renderer->Initialize(m_Rect.size);
	m_RendererInitialized = true;
}

void UIViewport::SetSize(ivec2 size)
{
	if (m_RendererInitialized)
	{
		m_Renderer->~UIViewportRenderer();
		m_Renderer = new(m_Renderer) UIViewportRenderer();
	}
	m_Rect.size = size;
	if(m_RendererInitialized)
		m_Renderer->Initialize(m_Rect.size);
}

bool UIViewport::Draw(uint16 level) 
{
	UNUSED(level);

	m_Renderer->Draw(m_WorldPos + m_Rect.pos, m_Rect.size);
	
	return false;
}

UIViewportRenderer::~UIViewportRenderer()
{
	if (!m_Initialized)return;
	delete m_pTex; m_pTex = nullptr;
	glDeleteFramebuffers(1, &m_FBO);
}

void UIViewportRenderer::Draw(ivec2 pos, ivec2 size)
{
	if (!m_Initialized)return;
	STATE->SetShader(m_pShader);
	STATE->SetViewport(pos, size);

	STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_pTex->GetHandle());
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}

void UIViewportRenderer::Initialize(ivec2 size)
{
	m_pShader = ContentManager::Load<ShaderData>("Shaders/EditorComposite.glsl");

	STATE->SetShader(m_pShader);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTex"), 0);

	TextureParameters params = TextureParameters();
	params.minFilter = GL_NEAREST;
	params.magFilter = GL_NEAREST;

	glGenFramebuffers(1, &m_FBO);
	STATE->BindFramebuffer(m_FBO);
	m_pTex = new TextureData(size.x, size.y, GL_RGB16F, GL_RGB, GL_FLOAT);
	m_pTex->Build();
	m_pTex->SetParameters(params);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pTex->GetHandle(), 0);

	m_Initialized = true;
}
