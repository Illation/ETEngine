#include "stdafx.h"
#include "GuiApplication.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <EtCore/UpdateCycle/PerformanceInfo.h>

#include <EtGUI/Context/RmlGlobal.h>

#include <EtApplication/Core/PackageResourceManager.h>

#include "GuiWindow.h"


namespace et {
namespace app {


//=================
// GUI Application
//=================


//-------------------------------
// GuiApplication::ReceiveEvents
//
// Sleep until there are any events up until a timout, in milliseconds, or infinitely if timeout is set to -1
//
void GuiApplication::ReceiveEvents(int64 const timeout)
{
#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	core::PerformanceInfo::GetInstance()->Update();
#endif

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

//--------------------------------
// GuiApplication::RegisterWindow
//
void GuiApplication::RegisterWindow(Ptr<GuiWindow> const window)
{
	ET_ASSERT(std::find(m_Windows.cbegin(), m_Windows.cend(), window) == m_Windows.cend());

	if (m_Windows.empty())
	{
		rhi::ContextHolder::Instance().CreateMainRenderContext(ToPtr(&window->GetRenderWindow())); // also initializes the viewport and its renderer

		gui::RmlGlobal::GetInstance()->SetCursorShapeManager(window);
		gui::RmlGlobal::GetInstance()->SetClipboardController(window);

		core::ResourceManager::SetInstance(std::move(Create<app::PackageResourceManager>()));

		core::ContextManager::GetInstance()->SetActiveContext(&m_Context);
		m_Context.time->Start();

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
		core::PerformanceInfo::GetInstance()->StartFrameTimer();
#endif
	}

	m_Windows.push_back(window);
}

//----------------------------------
// GuiApplication::UnregisterWindow
//
void GuiApplication::UnregisterWindow(GuiWindow const* const window)
{
	auto const foundIt = std::find_if(m_Windows.begin(), m_Windows.end(), [window](Ptr<GuiWindow> const el)
		{
			return (el.Get() == window);
		});
	ET_ASSERT(foundIt != m_Windows.cend());

	core::RemoveSwap(m_Windows, foundIt);

	if (m_Windows.empty())
	{
		gui::RmlGlobal::Destroy();
		core::ResourceManager::DestroyInstance();
	}
}


} // namespace app
} // namespace et



