#include "stdafx.h"
#include "GlfwEventManager.h"

#include <EtCore/Input/InputManager.h>


namespace et {
namespace app {


//=====================
// GLFW Event Manager
//=====================


//----------------------------
// GlfwEventManager::Init
//
// Should be called after we have an input manager, and after GLFW gave us a window
// Register for all events we may be interested in
// Init all possible cursors, and register as a cursor manager with the input manager
//
void GlfwEventManager::Init(Ptr<GlfwRenderArea> const renderArea)
{
	InitEvents(renderArea);

	// window closing
	glfwSetWindowCloseCallback(renderArea->GetWindow(), [](GLFWwindow* const)
		{
			core::InputManager::GetInstance()->Quit();
		});
}

//----------------------------
// GlfwEventManager::OnTick
//
// Pump the GLFW message loop into the input manager
//
void GlfwEventManager::OnTick()
{
	glfwPollEvents();
	if (glfwWindowShouldClose(GetRenderArea()->GetWindow()))
	{
		core::InputManager::GetInstance()->Quit();
	}
}


} // namespace app
} // namespace et
						   