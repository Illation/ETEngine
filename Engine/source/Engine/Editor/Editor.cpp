#include "stdafx.h"
#include "Editor.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Base/TickOrder.h>
#include <Engine/UI/UIViewport.h>
#include <Engine/UI/UIContainer.h>
#include <Engine/UI/UIComponent.h>
#include <Engine/UI/UISplitter.h>
#include <Engine/UI/UIPadding.h>
#include <Engine/GraphicsHelper/SpriteRenderer.h>
#include <Engine/GraphicsHelper/TextRenderer.h>
#include <Engine/GraphicsHelper/DebugRenderer.h>


#ifdef EDITOR

Editor::Editor()
	: I_Tickable(static_cast<uint32>(E_TickOrder::TICK_Editor))
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
	m_EditorFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Consolas_32.fnt"_hash);

	//Create toolbar
	m_pToolbar->SetColor(vec4(vec3(0.2f), 1));
		UIDynamicBox* pDynBox = new UIDynamicBox(UIDynamicBox::Mode::VERTICAL);
			pDynBox->AddChild(new UIText("Test Text", m_EditorFont), UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(new UIText("Another text line", m_EditorFont), UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(
				//new UIDynamicPadding(
					new UIText("and another", m_EditorFont)
					//, ivec4(5), vec4(vec3(0.35f), 1))
				, UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(new UISprite(ResourceManager::Instance()->GetAssetData<TextureData>("starSprite.png"_hash)), 
				UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(new UIText("and another", m_EditorFont), UIDynamicBox::Positioning::DYNAMIC);
			pDynBox->AddChild(new UISprite( ResourceManager::Instance()->GetAssetData<TextureData>("sample_texture.png"_hash)), 
				UIDynamicBox::Positioning::DYNAMIC);
	m_pToolbar->SetChild(pDynBox);

	CalculateViewportSize(WINDOW.EditorDimensions);

	m_Viewport->Initialize();
}

void Editor::Update()
{
	//process input here
	m_Root->Update();
}

void Editor::DrawSceneVisualizers()
{
	DebugRenderer::GetInstance()->DrawGrid(100.f);
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
