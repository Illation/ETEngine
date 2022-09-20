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

	glfwSetWindowFocusCallback(m_RenderWindow.GetArea().GetWindow(), [](GLFWwindow* const window, int const focused)
		{
			GuiWindow* const guiWindow = static_cast<GuiWindow*>(glfwGetWindowUserPointer(window)); 
			guiWindow->m_EventDispatcher.Notify((focused == 0) ? GuiWindow::GW_LooseFocus : GuiWindow::GW_GainFocus, new GuiWindow::EventData(guiWindow));
		});

	glfwSetWindowMaximizeCallback(m_RenderWindow.GetArea().GetWindow(), [](GLFWwindow* const window, int const maxed)
		{
			GuiWindow* const guiWindow = static_cast<GuiWindow*>(glfwGetWindowUserPointer(window)); 
			guiWindow->m_EventDispatcher.Notify((maxed == 0) ? GuiWindow::GW_Restore : GuiWindow::GW_Maximize, new GuiWindow::EventData(guiWindow));
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

//-----------------------------
// GuiWindow::RegisterCallback
//
GuiWindow::T_EventCallbackId GuiWindow::RegisterCallback(T_EventFlags const flags, T_EventCallback& callback)
{
	return m_EventDispatcher.Register(flags, callback);
}

//-------------------------------
// GuiWindow::UnregisterCallback
//
void GuiWindow::UnregisterCallback(T_EventCallbackId& callbackId)
{
	m_EventDispatcher.Unregister(callbackId);
}

//----------------------
// GuiWindow::StartDrag
//
void GuiWindow::StartDrag()
{
	m_RenderWindow.StartDrag();
}

//--------------------------------
// GuiWindow::SetCustomBorderSize
//
void GuiWindow::SetCustomBorderSize(int32 const size)
{
	m_CustomBorderSize = size;
	ivec2 dim = m_RenderWindow.GetSettings().GetSize();
	dim.y += m_CustomBorderSize;
	m_RenderWindow.SetDimensions(dim);
}

//----------------------
// GuiWindow::Maximized
//
bool GuiWindow::Focused() const
{
	return (glfwGetWindowAttrib(m_RenderWindow.GetArea().GetWindow(), GLFW_FOCUSED) != 0);
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

//---------------------
// GuiWindow::GetTitle
//
std::string const& GuiWindow::GetTitle() const
{
	return m_RenderWindow.GetSettings().m_Title;
}


} // namespace app
} // namespace et

