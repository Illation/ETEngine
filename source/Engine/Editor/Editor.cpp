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
	m_pRenderer = nullptr;
}

void Editor::Initialize()
{
	CalculateViewportSize(WINDOW.EditorDimensions);

	m_pRenderer->Initialize();

	UISprite testSprite = UISprite();
	testSprite.rect = iRect( ivec2( 0, 0 ), ivec2( 200, 200 ) );
	testSprite.color = vec4( 0.2f );
	testSprite.texture = nullptr;//ContentManager::Load<TextureData>( "Resources/Textures/sample.png" );
	m_UISprites.push_back( testSprite );
}

void Editor::Update()
{
//process input here
}

void Editor::Draw()
{
	m_pRenderer->Draw(m_RedrawUI);
}

GLuint Editor::GetSceneTarget()
{
	return m_pRenderer->GetSceneTarget();
}

void Editor::CalculateViewportSize(ivec2 FullWindowDimensions)
{
	m_Viewport = iRect();
	m_Viewport.pos = ivec2( (int32)m_ToolbarSeparator, 0 );
	m_Viewport.size = ivec2( FullWindowDimensions.x - (int32)m_ToolbarSeparator, FullWindowDimensions.y );
}

void Editor::OnWindowResize(ivec2 EditorDimensions)
{
	CalculateViewportSize(EditorDimensions);

	m_pRenderer->~EditorRenderer();
	m_pRenderer = new(m_pRenderer) EditorRenderer();
	m_pRenderer->Initialize();
}

#endif
