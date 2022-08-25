#include "stdafx.h"
#include "GuiApplication.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <EtGUI/Context/RmlGlobal.h>

#include <EtApplication/Core/PackageResourceManager.h>

#include "GuiWindow.h"


namespace et {
namespace app {


//=================
// GUI Application
//=================


//-------------------------------
// GuiApplication::WaitForEvents
//
// Sleep until there are any events up until a timout, in milliseconds, or infinitely if timeout is set to 0
//
void GuiApplication::WaitForEvents(uint64 const timeout)
{
	if (timeout == 0u)
	{
		glfwWaitEvents();
	}
	else
	{
		glfwWaitEventsTimeout(static_cast<double>(timeout) / 1000.0);
	}
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



