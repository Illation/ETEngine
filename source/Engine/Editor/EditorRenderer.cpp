#include "stdafx.hpp"
#include "EditorRenderer.hpp"
#include "Editor.hpp"
#include "SpriteRenderer.hpp"
#include "TextRenderer.hpp"
#include "PrimitiveRenderer.hpp"
#include "TextureData.hpp"
#include "ShaderData.hpp"

#ifdef EDITOR

EditorRenderer::EditorRenderer()
{

}

EditorRenderer::~EditorRenderer()
{
	delete m_SceneTex; m_SceneTex = nullptr;
	glDeleteFramebuffers( 1, &m_SceneFBO );

	delete m_UITex; m_UITex = nullptr;
	glDeleteFramebuffers( 1, &m_UIFBO );
}

void EditorRenderer::Initialize()
{
	m_EditorUIShader = ContentManager::Load<ShaderData>( "Shaders/EditorComposite.glsl" );

	STATE->SetShader( m_EditorUIShader );
	glUniform1i( glGetUniformLocation( m_EditorUIShader->GetProgram(), "uTex" ), 0 );
	m_uSize = glGetUniformLocation( m_EditorUIShader->GetProgram(), "uSize" );
	m_uOffset = glGetUniformLocation( m_EditorUIShader->GetProgram(), "uOffset" );

	CreateFramebuffers();
}

void EditorRenderer::Draw(bool redrawUI)
{
	if(redrawUI)
	{
		DrawUI();
	}

	STATE->BindFramebuffer( 0 );
	STATE->SetShader( m_EditorUIShader );

	glUniform2f( m_uSize, 1.f, 1.f );
	glUniform2f( m_uOffset, 0.f, 0.f );
	STATE->LazyBindTexture( 0, GL_TEXTURE_2D, m_UITex->GetHandle() );
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	ivec2 windowDim = WINDOW.Dimensions;
	iRect viewport = Editor::GetInstance()->GetViewport();
	ivec2 offset = viewport.pos / windowDim;
	ivec2 size = viewport.size / windowDim;
	glUniform2f( m_uSize, (float)offset.x, (float)offset.y );
	glUniform2f( m_uOffset, (float)offset.x, (float)offset.y );
	STATE->LazyBindTexture( 0, GL_TEXTURE_2D, m_SceneTex->GetHandle() );
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();
}

GLuint EditorRenderer::GetSceneTarget()
{
	return m_SceneFBO;
}

void EditorRenderer::DrawUI()
{
	STATE->BindFramebuffer( m_UIFBO );

	for(auto sprite : Editor::GetInstance()->GetSprites())
	{
		SpriteRenderer::GetInstance()->Draw( sprite.texture, vec2( (float)sprite.rect.pos.x, (float)sprite.rect.pos.y ), sprite.color,
											 vec2(0), vec2( (float)sprite.rect.size.x, (float)sprite.rect.size.y ), 0, 1 );
	}

	SpriteRenderer::GetInstance()->Draw();
	TextRenderer::GetInstance()->Draw();
}

void EditorRenderer::CreateFramebuffers()
{
	ivec2 size = Editor::GetInstance()->GetViewport().size;

	TextureParameters params = TextureParameters();
	params.minFilter = GL_NEAREST;
	params.magFilter = GL_NEAREST;
	params.wrapS = GL_CLAMP_TO_EDGE;
	params.wrapT = GL_CLAMP_TO_EDGE;

	glGenFramebuffers( 1, &m_SceneFBO );
	STATE->BindFramebuffer( m_SceneFBO );
	m_SceneTex = new TextureData( size.x, size.y, GL_RGB16F, GL_RGB, GL_FLOAT );
	m_SceneTex->Build();
	m_SceneTex->SetParameters( params );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SceneTex->GetHandle(), 0 );

	glGenFramebuffers( 1, &m_UIFBO );
	STATE->BindFramebuffer( m_UIFBO );
	m_UITex = new TextureData( WINDOW.Width, WINDOW.Height, GL_RGB16F, GL_RGB, GL_FLOAT );
	m_UITex->Build();
	m_UITex->SetParameters( params );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_UITex->GetHandle(), 0 );
}

#endif
