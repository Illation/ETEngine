#include "stdafx.hpp"
#include "UIViewport.h"
#include "PrimitiveRenderer.hpp"
#include "TextureData.hpp"
#include "ShaderData.hpp"

UIViewport::UIViewport():UIContainer()
{
}

UIViewport::~UIViewport()
{
	delete m_pTex; m_pTex = nullptr;
	glDeleteFramebuffers(1, &m_FBO);
}

GLuint UIViewport::GetTarget() const
{
	return m_FBO;
}

void UIViewport::Initialize()
{
	m_pShader = ContentManager::Load<ShaderData>("Shaders/EditorComposite.glsl");

	STATE->SetShader(m_pShader);
	glUniform1i(glGetUniformLocation(m_pShader->GetProgram(), "uTex"), 0);

	CreateFramebuffers();
}

bool UIViewport::Draw(uint16 level) 
{
	UNUSED(level);

	STATE->SetShader(m_pShader);
	STATE->SetViewport(m_WorldPos + m_Rect.pos, m_Rect.size);

	STATE->LazyBindTexture(0, GL_TEXTURE_2D, m_pTex->GetHandle());
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
	
	return false;
}

iRect UIViewport::CalculateDimensions(const ivec2 &worldPos)
{
	m_WorldPos = worldPos;
	iRect ret = m_Rect;
	ret.pos = ret.pos + m_WorldPos;
	return ret;
}

void UIViewport::SetSize(ivec2 size)
{
	m_Rect.size = size;
}

void UIViewport::CreateFramebuffers()
{
	TextureParameters params = TextureParameters();
	params.minFilter = GL_NEAREST;
	params.magFilter = GL_NEAREST;

	glGenFramebuffers(1, &m_FBO);
	STATE->BindFramebuffer(m_FBO);
	m_pTex = new TextureData(m_Rect.size.x, m_Rect.size.y, GL_RGB16F, GL_RGB, GL_FLOAT);
	m_pTex->Build();
	m_pTex->SetParameters(params);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pTex->GetHandle(), 0);

	m_HasFBO = true;
}
