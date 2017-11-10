#include "stdafx.hpp"
#include "Editor.hpp"

#include "EditorRenderer.hpp"

#ifdef EDITOR

Editor::Editor()
{
	m_pRenderer = new EditorRenderer();
}

Editor::~Editor()
{
	delete m_pRenderer;
}

void Editor::Initialize()
{
	m_Viewport = iRect();
	m_Viewport.pos = ivec2( (int32)m_ToolbarSeparator, 0 );
	m_Viewport.size = ivec2( WINDOW.Width - (int32)m_ToolbarSeparator, WINDOW.Height );

	m_pRenderer->Initialize();

	UISprite testSprite = UISprite();
	testSprite.rect = iRect( ivec2( 0, 0 ), ivec2( 1, 1 ) );
	testSprite.color = vec4( 1 );
	testSprite.texture = ContentManager::Load<TextureData>( "Resources/Textures/sample.png" );
	m_UISprites.push_back( testSprite );
}

void Editor::Update()
{

}

void Editor::Draw()
{
	m_pRenderer->Draw(m_RedrawUI);
}

GLuint Editor::GetSceneTarget()
{
	return m_pRenderer->GetSceneTarget();
}

void Editor::OnWindowResize()
{
	m_pRenderer->~EditorRenderer();
	m_pRenderer = new(m_pRenderer) EditorRenderer();
	m_pRenderer->Initialize();
}

#endif
