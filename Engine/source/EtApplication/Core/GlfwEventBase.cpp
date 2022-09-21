#include "stdafx.h"
#include "GlfwEventBase.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


namespace et {
namespace app {


//=================
// GLFW Event Base
//=================


//----------------------------------
// GlfwEventBase::GetButtonFromGlfw
//
E_MouseButton GlfwEventBase::GetButtonFromGlfw(int32 const bottonCode)
{
	switch (bottonCode)
	{
	case GLFW_MOUSE_BUTTON_LEFT: return E_MouseButton::Left;
	case GLFW_MOUSE_BUTTON_MIDDLE: return E_MouseButton::Center;
	case GLFW_MOUSE_BUTTON_RIGHT: return E_MouseButton::Right;

	default: return E_MouseButton::COUNT; // invalid button code
	}
}

GlfwEventBase::~GlfwEventBase()
{
	for (auto it = m_CursorMap.begin(); it != m_CursorMap.end(); ++it)
	{
		glfwDestroyCursor(it->second.Get());
	}
}

//---------------------------
// GlfwEventBase::InitEvents
//
void GlfwEventBase::InitEvents(Ptr<GlfwRenderArea> const renderArea)
{
	m_RenderArea = renderArea;

	glfwSetWindowUserPointer(m_RenderArea->GetWindow(), this);

	glfwSetKeyCallback(m_RenderArea->GetWindow(),
		[](GLFWwindow* const window, int32 const key, int32 const scancode, int32 const action, int32 const mods)
		{
			ET_UNUSED(scancode);

			GlfwEventBase* const eventBase = static_cast<GlfwEventBase*>(glfwGetWindowUserPointer(window));

			core::T_KeyModifierFlags const modifiers = static_cast<core::T_KeyModifierFlags>(mods);
			eventBase->SetCurrentModifiers(mods);

			core::RawInputProvider& inputProvider = eventBase->GetInputProvider();
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
	glfwSetCharCallback(m_RenderArea->GetWindow(),
		[](GLFWwindow* const window, uint32 const codepoint)
		{
			static_cast<GlfwEventBase*>(glfwGetWindowUserPointer(window))->GetInputProvider().IterateListeners(core::RawInputProvider::T_EventFn(
				[codepoint](core::I_RawInputListener& listener)
				{
					return listener.ProcessTextInput(static_cast<core::E_Character>(codepoint));
				}));
		});

	// Mouse clicking
	glfwSetMouseButtonCallback(m_RenderArea->GetWindow(), [](GLFWwindow* const window, int32 const button, int32 const action, int32 const mods)
		{
			core::T_KeyModifierFlags const modifiers = static_cast<core::T_KeyModifierFlags>(mods);

			core::RawInputProvider& inputProvider = static_cast<GlfwEventBase*>(glfwGetWindowUserPointer(window))->GetInputProvider();
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
	glfwSetCursorPosCallback(m_RenderArea->GetWindow(), [](GLFWwindow* const window, double const xpos, double const ypos)
		{
			GlfwEventBase* const eventBase = static_cast<GlfwEventBase*>(glfwGetWindowUserPointer(window));

			core::T_KeyModifierFlags const mods = eventBase->GetCurrentModifiers();
			core::RawInputProvider& inputProvider = eventBase->GetInputProvider();
			inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([xpos, ypos, mods](core::I_RawInputListener& listener)
				{
					return listener.ProcessMouseMove(math::vecCast<int32>(dvec2(xpos, ypos)), mods);
				}));
		});

	// scrolling
	glfwSetScrollCallback(m_RenderArea->GetWindow(), [](GLFWwindow* const window, double const xoffset, double const yoffset)
		{
			GlfwEventBase* const eventBase = static_cast<GlfwEventBase*>(glfwGetWindowUserPointer(window));

			core::T_KeyModifierFlags const mods = eventBase->GetCurrentModifiers();
			core::RawInputProvider& inputProvider = eventBase->GetInputProvider();
			inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([xoffset, yoffset, mods](core::I_RawInputListener& listener)
				{
					return listener.ProcessMouseWheelDelta(math::vecCast<int32>(dvec2(xoffset, yoffset)), mods);
				}));
		});

	// cursor enters or leaves the window
	glfwSetCursorEnterCallback(m_RenderArea->GetWindow(), [](GLFWwindow* const window, int32 const entered)
		{
			GlfwEventBase* const eventBase = static_cast<GlfwEventBase*>(glfwGetWindowUserPointer(window));

			core::T_KeyModifierFlags const mods = eventBase->GetCurrentModifiers();
			core::RawInputProvider& inputProvider = eventBase->GetInputProvider();
			inputProvider.IterateListeners(core::RawInputProvider::T_EventFn([mods, entered](core::I_RawInputListener& listener)
				{
					return listener.ProcessMouseEnterLeave(entered == GLFW_TRUE ? true : false, mods);
				}));
		});

	// window resizing
	glfwSetWindowSizeCallback(renderArea->GetWindow(), [](GLFWwindow* const window, int32 const width, int32 const height)
		{
			GlfwEventBase* const eventBase = static_cast<GlfwEventBase*>(glfwGetWindowUserPointer(window));
			eventBase->m_RenderArea->SetSize(ivec2(width, height));
		});

	m_CursorMap[core::E_CursorShape::Arrow] = ToPtr(glfwCreateStandardCursor(GLFW_ARROW_CURSOR));
	m_CursorMap[core::E_CursorShape::IBeam] = ToPtr(glfwCreateStandardCursor(GLFW_IBEAM_CURSOR));
	m_CursorMap[core::E_CursorShape::Crosshair] = ToPtr(glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR));
	m_CursorMap[core::E_CursorShape::Hand] = ToPtr(glfwCreateStandardCursor(GLFW_HAND_CURSOR));
	m_CursorMap[core::E_CursorShape::SizeWE] = ToPtr(glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
	m_CursorMap[core::E_CursorShape::SizeNS] = ToPtr(glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR));
	m_CursorMap[core::E_CursorShape::SizeNWSE] = ToPtr(glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR));
	m_CursorMap[core::E_CursorShape::SizeNESW] = ToPtr(glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR));
	m_CursorMap[core::E_CursorShape::SizeAll] = ToPtr(glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR));
	m_CursorMap[core::E_CursorShape::NotAllowed] = ToPtr(glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR));
}

//-------------------------------
// GlfwEventBase::SetCursorShape
//
// Change the cursor currently used
//
bool GlfwEventBase::SetCursorShape(core::E_CursorShape const shape)
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
			glfwSetCursor(m_RenderArea->GetWindow(), it->second.Get());
			return true;
		}

		ET_TRACE_W(ET_CTX_APP, "INPUT::SetCursorShape > Shape not found");
		return false;
	}
}

//---------------------------------
// GlfwEventBase::SetClipboardText
//
void GlfwEventBase::SetClipboardText(std::string const& textUtf8)
{
	glfwSetClipboardString(m_RenderArea->GetWindow(), textUtf8.c_str());
}

//---------------------------------
// GlfwEventBase::GetClipboardText
//
void GlfwEventBase::GetClipboardText(std::string& outTextUtf8)
{
	outTextUtf8 = glfwGetClipboardString(m_RenderArea->GetWindow());
}


} // namespace app
} // namespace et
