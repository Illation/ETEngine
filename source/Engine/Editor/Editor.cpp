#include "stdafx.hpp"
#include "Editor.hpp"

#include "UI\UIViewport.h"
#include "UI\UIContainer.h"

#ifdef EDITOR

Editor::Editor()
{
	UISplitter* pSplitter = new UISplitter(UISplitter::Mode::HORIZONTAL);

		UIPortal* pPortal = new UIPortal();
		pPortal->SetColor(vec4(vec3(0.5f), 1));
	pSplitter->SetFirst(pPortal);

		m_Viewport = new UIViewport();
	pSplitter->SetSecond(m_Viewport);

	m_Root = pSplitter;
}

Editor::~Editor()
{
	delete m_Root;
	m_Root = nullptr;
}

void Editor::Initialize()
{
	CalculateViewportSize(WINDOW.EditorDimensions);

	m_Viewport->Initialize();
}

void Editor::Update()
{
//process input here
}

void Editor::Draw()
{
	STATE->BindFramebuffer(0);
	STATE->SetViewport(ivec2(0), WINDOW.EditorDimensions);
	m_Root->Draw(0);
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
