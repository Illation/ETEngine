#include "stdafx.h"
#include "GuiApplication.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <RmlUi/Core/Factory.h>

#include <EtCore/UpdateCycle/PerformanceInfo.h>

#include <EtRHI/Util/PrimitiveRenderer.h>

#include <EtGUI/Context/RmlGlobal.h>

#include <EtApplication/Core/PackageResourceManager.h>
#include <EtApplication/GUI/ElementWindow.h>

#include "GuiWindow.h"


namespace et {
namespace app {


//=================
// GUI Application
//=================


// static
Ptr<app::GuiApplication> GuiApplication::s_GlobalInstance;


//------------------------------------
// GuiApplication::HasRunningInstance
//
// returns true if there is a GuiApplication with active windows
//
bool GuiApplication::HasRunningInstance()
{
	return (s_GlobalInstance != nullptr);
}

//--------------------------
// GuiApplication::Instance
//
// Get a pointer to the single GuiApplication that has running windows
//
GuiApplication* GuiApplication::Instance()
{
	ET_ASSERT(HasRunningInstance());
	return s_GlobalInstance.Get();
}

//-----------------------
// GuiApplication::d-tor
//
GuiApplication::~GuiApplication()
{
	ET_ASSERT(!gui::RmlGlobal::IsInitialized());
}

//-------------------------------
// GuiApplication::ReceiveEvents
//
// Sleep until there are any events up until a timeout, in milliseconds, or infinitely if timeout is set to -1
//
void GuiApplication::ReceiveEvents(int64 const timeout)
{
#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	core::PerformanceInfo::GetInstance()->Update();
#endif

	CloseMarkedWindows();

	if (timeout == 0)
	{
		glfwPollEvents();
	}
	else if (timeout < 0)
	{
		glfwWaitEvents();
	}
	else
	{
		glfwWaitEventsTimeout(static_cast<double>(timeout) / 1000.0);
	}

	m_Context.time->Update();

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	core::PerformanceInfo::GetInstance()->StartFrameTimer();
#endif
}

//----------------------
// GuiApplication::Draw
//
void GuiApplication::Draw()
{
	for (Ptr<GuiWindow> const window : m_Windows)
	{
		window->GetRenderArea()->Update();
	}
}

//----------------------------
// GuiApplication::MakeWindow
//
// This initializes most required systems when the first window is made
//
Ptr<GuiWindow> GuiApplication::MakeWindow(core::WindowSettings const& settings)
{
	m_Windows.push_back(Create<GuiWindow>(settings));
	Ptr<GuiWindow> const window = ToPtr(m_Windows.back().Get());

	if (m_Windows.size() == 1u)
	{
		ET_ASSERT(s_GlobalInstance == nullptr, "Only one instance of a GuiApplication can be initialized with a window at a time");
		s_GlobalInstance = ToPtr(this);

		// also initializes the viewport and its renderer
		rhi::ContextHolder::Instance().CreateMainRenderContext(ToPtr(&window->GetRenderWindow()));

		ET_ASSERT(!gui::RmlGlobal::IsInitialized());

		gui::RmlGlobal::GetInstance()->SetCursorShapeManager(window);
		gui::RmlGlobal::GetInstance()->SetClipboardController(window);

		ET_ASSERT(gui::RmlGlobal::IsInitialized());
		Rml::RegisterPlugin(this);

		Rml::Factory::RegisterElementInstancer("window_handle", &m_HandleInstancer);
		Rml::Factory::RegisterElementInstancer("window", &m_WindowInstancer);

		core::ResourceManager::SetInstance(std::move(Create<app::PackageResourceManager>()));

		core::ContextManager::GetInstance()->SetActiveContext(&m_Context);
		m_Context.time->Start();

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
		core::PerformanceInfo::GetInstance()->StartFrameTimer();
#endif
	}

	window->Init();

	return window;
}

//------------------------------------
// GuiApplication::MarkWindowForClose
//
void GuiApplication::MarkWindowForClose(GuiWindow const* const window)
{
	auto const foundIt = std::find(m_Windows.begin(), m_Windows.end(), window);
	ET_ASSERT(foundIt != m_Windows.cend(), "Window is not registered");

	size_t const idx = static_cast<size_t>(foundIt - m_Windows.begin());
	if (std::find(m_WindowsToDelete.cbegin(), m_WindowsToDelete.cend(), idx) == m_WindowsToDelete.cend())
	{
		m_WindowsToDelete.push_back(idx);
	}
}

//---------------------------
// GuiApplication::GetWindow
//
Ptr<GuiWindow> GuiApplication::GetWindow(Rml::Context const* const context)
{
	auto const foundIt = std::find_if(m_Windows.begin(), m_Windows.end(), [context](UniquePtr<GuiWindow> const& window)
		{
			return (window->GetContext().GetImpl() == context);
		});

	if (foundIt != m_Windows.cend())
	{
		return ToPtr(foundIt->Get());
	}

	return nullptr;
}

//---------------------------------
// GuiApplication::OnElementCreate
//
void GuiApplication::OnElementCreate(Rml::Element* const element)
{
	GuiWindow const* const guiWindow = GetWindow(element->GetContext()).Get();
	if (guiWindow != nullptr)
	{
		ElementWindow::OnFocusChangeRecursive(element, guiWindow->Focused());
	}
}

//------------------------------------
// GuiApplication::CloseMarkedWindows
//
void GuiApplication::CloseMarkedWindows()
{
	if (m_WindowsToDelete.empty())
	{
		return;
	}

	bool destroyLast = false;
	for (size_t idx = m_WindowsToDelete.size() - 1; idx < m_WindowsToDelete.size(); --idx)
	{
		size_t const windowIdx = m_WindowsToDelete[idx];
		if (m_Windows.size() == 1u)
		{
			ET_ASSERT(windowIdx == 0u);
			destroyLast = true;

			m_Windows[windowIdx]->Deinit();
		}
		else
		{
			core::RemoveSwap(m_Windows, m_Windows.begin() + windowIdx);
		}
	}

	m_WindowsToDelete.clear();

	if (destroyLast)
	{
		gui::RmlGlobal::Destroy();
		core::ResourceManager::DestroyInstance();

		rhi::PrimitiveRenderer::Instance().Deinit();

		m_Windows.clear();

		s_GlobalInstance = nullptr;
	}
}


} // namespace app
} // namespace et



