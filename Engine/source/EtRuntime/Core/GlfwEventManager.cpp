#include "stdafx.h"
#include "GlfwEventManager.h"

#include <EtCore/Input/InputManager.h>


namespace et {
namespace rt {


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
void GlfwEventManager::Init(Ptr<GlfwRenderArea> const renderArea)
{
	m_RenderArea = renderArea;

	// Register for keyboard events
	glfwSetKeyCallback(renderArea->GetWindow(), 
		[](GLFWwindow* const window, int32 const key, int32 const scancode, int32 const action, int32 const mods)
		{
			UNUSED(window);
			UNUSED(scancode);

			GlfwEventManager* const inst = GlfwEventManager::GetInstance();

			core::T_KeyModifierFlags const modifiers = static_cast<core::T_KeyModifierFlags>(mods);
			inst->SetCurrentModifiers(mods);

			core::RawInputProvider& inputProvider = inst->GetInputProvider();
			if (action == GLFW_PRESS)
			{
				inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([key, modifiers](core::I_RawInputListener& listener)
					{
						return listener.ProcessKeyPressed(static_cast<E_KbdKey>(key), modifiers);
					}));
			}
			else if (action == GLFW_RELEASE)
			{
				inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([key, modifiers](core::I_RawInputListener& listener)
					{
						return listener.ProcessKeyReleased(static_cast<E_KbdKey>(key), modifiers);
					}));
			}
		});

	// Register for text events
	glfwSetCharCallback(renderArea->GetWindow(),
		[](GLFWwindow* const window, uint32 const codepoint)
		{
			UNUSED(window);

			GlfwEventManager::GetInstance()->GetInputProvider().IterateListeners(core::RawInputProvider::T_EventFn(
				[codepoint](core::I_RawInputListener& listener)
				{
					return listener.ProcessTextInput(static_cast<core::E_Character>(codepoint));
				}));
		});

	// Mouse clicking
	glfwSetMouseButtonCallback(renderArea->GetWindow(), [](GLFWwindow* const window, int32 const button, int32 const action, int32 const mods)
		{
			UNUSED(window);

			core::T_KeyModifierFlags const modifiers = static_cast<core::T_KeyModifierFlags>(mods);

			core::RawInputProvider& inputProvider = GlfwEventManager::GetInstance()->GetInputProvider();
			if (action == GLFW_PRESS)
			{
				inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([button, modifiers](core::I_RawInputListener& listener)
					{
						return listener.ProcessMousePressed(GetButtonFromGlfw(button), modifiers);
					}));
			}
			else if (action == GLFW_RELEASE)
			{
				inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([button, modifiers](core::I_RawInputListener& listener)
					{
						return listener.ProcessMouseReleased(GetButtonFromGlfw(button), modifiers);
					}));
			}
		});

	// mouse motion #todo: support raw mouse motion
	glfwSetCursorPosCallback(renderArea->GetWindow(), [](GLFWwindow* const window, double const xpos, double const ypos)
		{
			UNUSED(window);

			GlfwEventManager* const inst = GlfwEventManager::GetInstance();
			core::T_KeyModifierFlags const mods = inst->GetCurrentModifiers();
			core::RawInputProvider& inputProvider = inst->GetInputProvider();
			inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([xpos, ypos, mods](core::I_RawInputListener& listener)
				{
					return listener.ProcessMouseMove(math::vecCast<int32>(dvec2(xpos, ypos)), mods);
				}));
		});

	// scrolling
	glfwSetScrollCallback(renderArea->GetWindow(), [](GLFWwindow* const window, double const xoffset, double const yoffset)
		{
			UNUSED(window);

			GlfwEventManager* const inst = GlfwEventManager::GetInstance();
			core::T_KeyModifierFlags const mods = inst->GetCurrentModifiers();
			core::RawInputProvider& inputProvider = inst->GetInputProvider();
			inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([xoffset, yoffset, mods](core::I_RawInputListener& listener)
				{
					return listener.ProcessMouseWheelDelta(math::vecCast<int32>(dvec2(xoffset, yoffset)), mods);
				}));
		});

	// window resizing
	glfwSetWindowSizeCallback(renderArea->GetWindow(), [](GLFWwindow* const window, int32 const width, int32 const height)
		{
			UNUSED(window);

			GlfwRenderArea* const renderArea = static_cast<GlfwRenderArea*>(glfwGetWindowUserPointer(window));

			renderArea->SetSize(ivec2(width, height));
		});

	// window closing
	glfwSetWindowCloseCallback(renderArea->GetWindow(), [](GLFWwindow* const window)
		{
			core::InputManager::GetInstance()->Quit();
		});

	m_CursorMap[core::E_CursorShape::Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	m_CursorMap[core::E_CursorShape::IBeam] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	m_CursorMap[core::E_CursorShape::Crosshair] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	m_CursorMap[core::E_CursorShape::SizeWE] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	m_CursorMap[core::E_CursorShape::SizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	m_CursorMap[core::E_CursorShape::Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
}

//----------------------------
// GlfwEventManager::OnCursorResize
//
// Change the cursor currently used
//
bool GlfwEventManager::OnCursorResize(core::E_CursorShape const shape)
{
	if (shape == core::E_CursorShape::None)
	{
		glfwSetInputMode(m_RenderArea->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		return true;
	}
	else
	{
		glfwSetInputMode(m_RenderArea->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		auto it = m_CursorMap.find(shape);
		if (it != m_CursorMap.end())
		{
			glfwSetCursor(m_RenderArea->GetWindow(), it->second);
			return true;
		}

		LOG("INPUT::OnCursorResize > Shape not found", core::LogLevel::Warning);
		return false;
	}
}

//----------------------------
// GlfwEventManager::OnTick
//
// Pump the GLFW message loop into the input manager
//
void GlfwEventManager::OnTick()
{
	glfwPollEvents();
	if (glfwWindowShouldClose(m_RenderArea->GetWindow()))
	{
		core::InputManager::GetInstance()->Quit();
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


} // namespace rt
} // namespace et
						   