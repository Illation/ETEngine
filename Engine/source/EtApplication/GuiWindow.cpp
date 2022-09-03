#include "stdafx.h"
#include "GuiWindow.h"

#include <RmlUi/Core/ElementDocument.h>

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
{
	m_Viewport.SetTickDisabled(true);
	m_Viewport.SetRenderer(ToPtr(&m_GuiRenderer));
}

//------------------
// GuiWindow::d-tor
//
GuiWindow::~GuiWindow()
{
	Deinit();
	m_RenderWindow.GetArea().Uninitialize();
}

//-----------------
// GuiWindow::Init
//
void GuiWindow::Init()
{
	InitEvents(ToPtr(&m_RenderWindow.GetArea()));

	ET_ASSERT(glfwGetWindowUserPointer(m_RenderWindow.GetArea().GetWindow()) == this);
	glfwSetWindowCloseCallback(m_RenderWindow.GetArea().GetWindow(), [](GLFWwindow* const window)
		{
			GuiWindow const* const guiWindow = static_cast<GuiWindow*>(glfwGetWindowUserPointer(window)); // this pointer is set during InitEvents
			GuiApplication::Instance()->MarkWindowForClose(guiWindow);
		});

	m_Viewport.SetInputProvider(ToPtr(&GetInputProvider()));

	m_Viewport.SynchDimensions();
	m_Viewport.Redraw();

	m_GuiRenderer.Init(ToPtr(&GetInputProvider()));

	m_IsInitialized = true;
}

//-------------------
// GuiWindow::Deinit
//
void GuiWindow::Deinit()
{
	if (m_IsInitialized)
	{
		m_IsInitialized = false;

		m_GuiRenderer.Deinit();
		m_Viewport.SetRenderer(nullptr);
	}
}

//---------------------------
// GuiWindow::SetGuiDocument
//
void GuiWindow::SetGuiDocument(core::HashString const documentId)
{
	m_GuiRenderer.SetGuiDocument(documentId);

	Rml::ElementDocument* const doc = m_GuiRenderer.GetContext().GetDocument(documentId);
	ET_ASSERT(doc != nullptr);

	Rml::Element* const titleEl = doc->GetElementById("title");
	if (titleEl != nullptr)
	{
		titleEl->SetInnerRML(doc->GetTitle());
	}
}


} // namespace app
} // namespace et

