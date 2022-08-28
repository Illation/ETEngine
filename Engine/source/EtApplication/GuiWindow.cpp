#include "stdafx.h"
#include "GuiWindow.h"

#include "GuiApplication.h"


namespace et {
namespace app {


//============
// GUI Window
//============


//------------------
// GuiWindow::c-tor
//
GuiWindow::GuiWindow(core::WindowSettings const& settings, Ptr<GuiApplication> const application)
	: GlfwEventBase()
	, m_RenderWindow(settings)
	, m_Viewport(ToPtr(&m_RenderWindow.GetArea()))
	, m_App(application)
{
	m_Viewport.SetTickDisabled(true);
	m_Viewport.SetRenderer(ToPtr(&m_GuiRenderer));

	ET_ASSERT(m_App != nullptr);
	m_App->RegisterWindow(ToPtr(this));

	InitEvents(ToPtr(&m_RenderWindow.GetArea()));

	m_Viewport.SetInputProvider(ToPtr(&GetInputProvider()));

	m_Viewport.SynchDimensions();
	m_Viewport.Redraw();

	m_GuiRenderer.Init(ToPtr(&GetInputProvider()));
}

//------------------
// GuiWindow::d-tor
//
GuiWindow::~GuiWindow()
{
	m_GuiRenderer.Deinit();
	m_Viewport.SetRenderer(nullptr);

	m_RenderWindow.GetArea().Uninitialize();

	m_App->UnregisterWindow(this);
}

//---------------------------
// GuiWindow::SetGuiDocument
//
void GuiWindow::SetGuiDocument(core::HashString const documentId)
{
	m_GuiRenderer.SetGuiDocument(documentId);
}


} // namespace app
} // namespace et

