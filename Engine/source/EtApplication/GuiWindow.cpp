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
GuiWindow::GuiWindow(core::WindowSettings const& settings)
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

//------------------
// GuiWindow::Close
//
void GuiWindow::Close()
{
	GuiApplication::Instance()->MarkWindowForClose(this);
}

//----------------------------
// GuiWindow::ToggleMinimized
//
void GuiWindow::ToggleMinimized()
{
	if (Minimized())
	{
		if (Maximized())
		{
			glfwMaximizeWindow(m_RenderWindow.GetArea().GetWindow());
		}
		else
		{
			glfwRestoreWindow(m_RenderWindow.GetArea().GetWindow());
		}
	}
	else
	{
		glfwIconifyWindow(m_RenderWindow.GetArea().GetWindow());
	}
}

//----------------------------
// GuiWindow::ToggleMaximized
//
void GuiWindow::ToggleMaximized()
{
	if (Maximized())
	{
		glfwRestoreWindow(m_RenderWindow.GetArea().GetWindow());
	}
	else
	{
		glfwMaximizeWindow(m_RenderWindow.GetArea().GetWindow());
	}
}

//----------------------
// GuiWindow::StartDrag
//
void GuiWindow::StartDrag()
{
	m_RenderWindow.StartDrag();
}

//----------------------
// GuiWindow::Maximized
//
bool GuiWindow::Maximized() const
{
	return (glfwGetWindowAttrib(m_RenderWindow.GetArea().GetWindow(), GLFW_MAXIMIZED) != 0);
}

//----------------------
// GuiWindow::Minimized
//
bool GuiWindow::Minimized() const
{
	return (glfwGetWindowAttrib(m_RenderWindow.GetArea().GetWindow(), GLFW_ICONIFIED) != 0);
}


} // namespace app
} // namespace et

