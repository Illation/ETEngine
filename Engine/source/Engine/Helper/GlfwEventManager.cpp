#include "stdafx.h"
#include "GlfwEventManager.h"

#include <EtCore/Helper/InputManager.h>


//=====================
// GLFW Event Manager
//=====================


//----------------------------
// GlfwEventManager::d-tor
//
GlfwEventManager::~GlfwEventManager()
{
	for (auto it = m_CursorMap.begin(); it != m_CursorMap.end(); ++it)
	{
		glfwDestroyCursor(it->second);
	}
}

//----------------------------
// GlfwEventManager::Init
//
// Should be called after we have an input manager, and after GLFW gave us a window
// Register for all events we may be interested in
// Init all possible cursors, and register as a cursor manager with the input manager
//
void GlfwEventManager::Init(GLFWwindow* const window)
{
	m_Window = window;

	// Register for keyboard events
	glfwSetKeyCallback(m_Window, [](GLFWwindow* const window, int32 const key, int32 const scancode, int32 const action, int32 const mods)
	{
		UNUSED(window);
		UNUSED(scancode);
		UNUSED(mods);

		if (action == GLFW_PRESS)
		{
			InputManager::GetInstance()->OnKeyPressed(static_cast<E_KbdKey>(key));
		}
		else if (action == GLFW_RELEASE)
		{
			InputManager::GetInstance()->OnKeyReleased(static_cast<E_KbdKey>(key));
		}
	});

	// mouse motion #todo: support raw mouse motion
	glfwSetCursorPosCallback(m_Window, [](GLFWwindow* const window, double const xpos, double const ypos)
	{
		UNUSED(window);

		InputManager::GetInstance()->OnMouseMoved(etm::vecCast<int32>(dvec2(xpos, ypos)));
	});

	// Mouse clicking
	glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* const window, int32 const button, int32 const action, int32 const mods)
	{
		UNUSED(window);
		UNUSED(mods);

		if (action == GLFW_PRESS)
		{
			InputManager::GetInstance()->OnMousePressed(GetButtonFromGlfw(button));
		}
		else if (action == GLFW_RELEASE)
		{
			InputManager::GetInstance()->OnMouseReleased(GetButtonFromGlfw(button));
		}
	});

	// scrolling
	glfwSetScrollCallback(m_Window, [](GLFWwindow* const window, double const xoffset, double const yoffset)
	{
		UNUSED(window);

		InputManager::GetInstance()->SetMouseWheelDelta(etm::vecCast<int32>(dvec2(xoffset, yoffset)));
	});

	// window resizing
	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* const window, int32 const width, int32 const height)
	{
		UNUSED(window);

		WINDOW.Resize(width, height);
	});

	// window closing
	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* const window)
	{
		InputManager::GetInstance()->Quit();
	});

	m_CursorMap[E_CursorShape::Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_CursorMap[E_CursorShape::IBeam] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	m_CursorMap[E_CursorShape::Crosshair] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	m_CursorMap[E_CursorShape::SizeWE] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	m_CursorMap[E_CursorShape::SizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	m_CursorMap[E_CursorShape::Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

	InputManager::GetInstance()->RegisterCursorShapeManager(this);
}

//----------------------------
// GlfwEventManager::OnCursorResize
//
// Change the cursor currently used
//
bool GlfwEventManager::OnCursorResize(E_CursorShape const shape)
{
	auto it = m_CursorMap.find(shape);
	if (it != m_CursorMap.end())
	{
		glfwSetCursor(m_Window, it->second);
		return true;
	}

	LOG("INPUT::OnCursorResize > Shape not found", Warning);
	return false;
}

//----------------------------
// GlfwEventManager::OnTick
//
// Pump the GLFW message loop into the input manager
//
void GlfwEventManager::OnTick()
{
	glfwPollEvents();
	if (glfwWindowShouldClose(m_Window))
	{
		InputManager::GetInstance()->Quit();
	}
}

//--------------------------------------------
// GlfwEventManager::GetButtonFromGlfw
//
// Convert the events button to an E_MouseButton for the input manager
//
E_MouseButton GlfwEventManager::GetButtonFromGlfw(int32 const bottonCode)
{
	switch (bottonCode)
	{
	case GLFW_MOUSE_BUTTON_LEFT: return E_MouseButton::Left;
	case GLFW_MOUSE_BUTTON_MIDDLE: return E_MouseButton::Center;
	case GLFW_MOUSE_BUTTON_RIGHT: return E_MouseButton::Right;

	default: return E_MouseButton::COUNT; // invalid button code
	}			
}
						   