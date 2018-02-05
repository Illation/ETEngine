#include "stdafx.hpp"
#include "Editor.hpp"

#include "UI\UIViewport.h"
#include "UI\UIContainer.h"
#include "UI\UIComponent.hpp"
#include "UI\UISplitter.h"
#include "UI\UIPadding.h"
#include "SpriteRenderer.hpp"
#include "TextRenderer.hpp"

#ifdef EDITOR

Editor::Editor()
{
	UISplitter* pSplitter = new UISplitter(UISplitter::Mode::HORIZONTAL);

		m_pToolbar = new UIPortal();
	pSplitter->SetFirst(new UIFixedPadding(m_pToolbar, ivec4(5), vec4(vec3(0.15f), 1)));

		m_Viewport = new UIViewport();
	pSplitter->SetSecond(new UIFixedPadding(m_Viewport, ivec4(5), vec4(vec3(0.15f), 1)));

	m_Root = pSplitter;
}

Editor::~Editor()
{
	delete m_Root;
	m_Root = nullptr;
}

void Editor::Initialize()
{
	m_pEditorFont = ContentManager::Load<SpriteFont>("Resources/Fonts/Consolas_32.fnt");

	//Create toolbar
	m_pToolbar->SetColor(vec4(vec3(0.2f), 1));
		UIDynamicBox* pDynBox = new UIDynamicBox(UIDynamicBox::Mode::VERTICAL);
			pDynBox->AddChild(new UIText("Test Text", m_pEditorFont), UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(new UIText("Another text line", m_pEditorFont), UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(
				//new UIDynamicPadding(
					new UIText("and another", m_pEditorFont)
					//, ivec4(5), vec4(vec3(0.35f), 1))
				, UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(new UISprite(
				ContentManager::Load<TextureData>("Resources/Textures/starSprite.png")), UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(new UIText("and another", m_pEditorFont), UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(new UISprite(
				ContentManager::Load<TextureData>("Resources/Textures/sample.png")), UIDynamicBox::Positioning::DYNAMIC);
	m_pToolbar->SetChild(pDynBox);

	CalculateViewportSize(WINDOW.EditorDimensions);

	m_Viewport->Initialize();
}

void Editor::Update()
{
	//process input here
	m_Root->Update();
}

void Editor::Draw()
{
	STATE->BindFramebuffer(0);
	STATE->SetViewport(ivec2(0), WINDOW.EditorDimensions);
	uint16 level = 0;
	bool draw = true;
	while (draw)
	{
		draw = m_Root->Draw(level);
		level++;
		SpriteRenderer::GetInstance()->Draw();
		TextRenderer::GetInstance()->Draw();
	}
}

GLuint Editor::GetSceneTarget()
{
	return m_Viewport->GetTarget();
}

void Editor::CalculateViewportSize(ivec2 FullWindowDimensions, bool resizeBuffer)
{
	if (resizeBuffer)m_Root->SetSize(FullWindowDimensions);
	else m_Root->SetSizeOnly(FullWindowDimensions);
}

void Editor::OnWindowResize(ivec2 EditorDimensions)
{
	CalculateViewportSize(EditorDimensions, true);
}

#endif
