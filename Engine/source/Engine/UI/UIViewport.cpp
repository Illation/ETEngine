#include "stdafx.h"
#include "UIViewport.h"

#include <glad/glad.h>

#include <EtCore/Content/ResourceManager.h>

#include <Engine/GraphicsHelper/PrimitiveRenderer.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/Graphics/Shader.h>


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
	Viewport::GetCurrentApiContext()->DeleteFramebuffers(1, &m_FBO);
}

void UIViewportRenderer::Draw(ivec2 pos, ivec2 size)
{
	if (!m_Initialized)
	{
		return;
	}

	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->SetShader(m_pShader.get());
	api->SetViewport(pos, size);

	api->LazyBindTexture(0, GL_TEXTURE_2D, m_pTex->GetHandle());
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}

void UIViewportRenderer::Initialize(ivec2 size)
{
	GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_pShader = ResourceManager::Instance()->GetAssetData<ShaderData>("EditorComposite.glsl"_hash);

	api->SetShader(m_pShader.get());
	m_pShader->Upload("uTex"_hash, 0);

	TextureParameters params(false);

	api->GenFramebuffers(1, &m_FBO);
	api->BindFramebuffer(m_FBO);
	m_pTex = new TextureData(size.x, size.y, GL_RGB16F, GL_RGB, GL_FLOAT);
	m_pTex->Build();
	m_pTex->SetParameters(params);
	api->LinkTextureToFbo2D(0, GL_TEXTURE_2D, m_pTex->GetHandle(), 0);

	m_Initialized = true;
}
