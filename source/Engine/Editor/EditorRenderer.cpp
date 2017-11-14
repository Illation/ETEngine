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

	STATE->SetViewport( ivec2( 0 ), WINDOW.EditorDimensions );

	STATE->LazyBindTexture( 0, GL_TEXTURE_2D, m_UITex->GetHandle() );
	PrimitiveRenderer::GetInstance()->Draw<primitives::Quad>();

	iRect viewport = Editor::GetInstance()->GetViewport();
	STATE->SetViewport( viewport.pos, viewport.size );

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

	//temporary until ui system is in place
	SpriteRenderer::GetInstance()->Draw( nullptr, vec2( 0 ), vec4(0.2f), vec2(0), vec2( 200 ), 0, 1, SpriteScalingMode::TEXTURE );

	SpriteRenderer::GetInstance()->Draw();
	TextRenderer::GetInstance()->Draw();
}

void EditorRenderer::CreateFramebuffers()
{
	ivec2 dimensions = Editor::GetInstance()->GetViewport().size;

	TextureParameters params = TextureParameters();
	params.minFilter = GL_NEAREST;
	params.magFilter = GL_NEAREST;

	glGenFramebuffers( 1, &m_SceneFBO );
	STATE->BindFramebuffer( m_SceneFBO );
	m_SceneTex = new TextureData( dimensions.x, dimensions.y, GL_RGB16F, GL_RGB, GL_FLOAT );
	m_SceneTex->Build();
	m_SceneTex->SetParameters( params );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SceneTex->GetHandle(), 0 );

	glGenFramebuffers( 1, &m_UIFBO );
	STATE->BindFramebuffer( m_UIFBO );
	m_UITex = new TextureData( WINDOW.EditorWidth, WINDOW.EditorHeight, GL_RGB16F, GL_RGB, GL_FLOAT );
	m_UITex->Build();
	m_UITex->SetParameters( params );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_UITex->GetHandle(), 0 );
}

#endif
