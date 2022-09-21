#include "stdafx.h"
#include "ImguiPlatformBackend.h"

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include "ImGuiUtil.h"

#include <EtCore/Input/RawInputProvider.h>


namespace et {
namespace gui {


//========================
// ImGui Platform Backend
//========================


//------------------------------------
// ImguiPlatformBackend::AccessFromIO
//
// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
//
ImguiPlatformBackend* ImguiPlatformBackend::AccessFromIO()
{
	return ImGui::GetCurrentContext() ? static_cast<ImguiPlatformBackend*>(ImGui::GetIO().BackendPlatformUserData) : nullptr;
}


//----------------------------
// ImguiPlatformBackend::Init
//
void ImguiPlatformBackend::Init(Ptr<core::I_CursorShapeManager> const cursorManager,
	Ptr<core::I_ClipboardController> const clipboardController,
	Ptr<rhi::Viewport> const viewport)
{
	m_CursorShapeManager = cursorManager;
	m_ClipboardController = clipboardController;
	m_Viewport = viewport;

	ImGuiIO& io = ImGui::GetIO();

	ET_ASSERT(io.BackendPlatformUserData == nullptr);
	io.BackendPlatformUserData = static_cast<void*>(this);
	io.BackendPlatformName = "imgui_impl_etengine";
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	//io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)

	io.SetClipboardTextFn = [](void* userData, const char* text)
		{
			ImguiPlatformBackend* const backend = static_cast<ImguiPlatformBackend*>(userData);
			backend->m_ClipboardController->SetClipboardText(std::string(text));
		};

	io.GetClipboardTextFn = [](void* userData) -> char const*
		{
			ImguiPlatformBackend* const backend = static_cast<ImguiPlatformBackend*>(userData);
			backend->m_ClipboardController->GetClipboardText(backend->m_LastClipboardText);
			return backend->m_LastClipboardText.c_str();
		};

	io.ClipboardUserData = static_cast<void*>(this);

	m_Viewport->GetInputProvider()->RegisterListener(ToPtr(this));
}

//------------------------------
// ImguiPlatformBackend::Deinit
//
void ImguiPlatformBackend::Deinit()
{
	ImGuiIO& io = ImGui::GetIO();

	io.BackendPlatformName = nullptr;
	io.BackendPlatformUserData = nullptr;

	m_Viewport->GetInputProvider()->UnregisterListener(this);
}

//------------------------------
// ImguiPlatformBackend::Update
//
void ImguiPlatformBackend::Update()
{
	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = math::vecCast<float>(m_Viewport->GetDimensions());
	io.DisplayFramebufferScale = ImVec2(1.f, 1.f); // if we want to support dynamic framebuffer rescaling we need to take that into account here

	io.DeltaTime = core::ContextManager::GetInstance()->GetActiveContext()->time->DeltaTime();

	//UpdateMouseData(); // the imgui implementation for glfw here sets the mouse pos and updates the last valid mouse pos
	UpdateMouseCursor();

	// #todo: this is where we would update gamepads too
}


//-----------------------------------------
// ImguiPlatformBackend::ProcessKeyPressed
//
bool ImguiPlatformBackend::ProcessKeyPressed(E_KbdKey const key, core::T_KeyModifierFlags const modifiers)
{
	// #todo: if events are coming from GLFW, we might need to retranslate untranslated keys
	// #todo: might have to decode modifiers from the key for X11

	UpdateKeyModifiers(modifiers);

	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(ImguiUtil::GetKey(key), true);

	return io.WantCaptureKeyboard;
}

//------------------------------------------
// ImguiPlatformBackend::ProcessKeyReleased
//
bool ImguiPlatformBackend::ProcessKeyReleased(E_KbdKey const key, core::T_KeyModifierFlags const modifiers)
{
	UpdateKeyModifiers(modifiers);

	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(ImguiUtil::GetKey(key), false);

	return io.WantCaptureKeyboard;
}

//-------------------------------------------
// ImguiPlatformBackend::ProcessMousePressed
//
bool ImguiPlatformBackend::ProcessMousePressed(E_MouseButton const button, core::T_KeyModifierFlags const modifiers)
{
	UpdateKeyModifiers(modifiers);

	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(ImguiUtil::GetButton(button), true);

	return io.WantCaptureMouse; 
}

//--------------------------------------------
// ImguiPlatformBackend::ProcessMouseReleased
//
bool ImguiPlatformBackend::ProcessMouseReleased(E_MouseButton const button, core::T_KeyModifierFlags const modifiers)
{
	UpdateKeyModifiers(modifiers);

	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(ImguiUtil::GetButton(button), false);

	return io.WantCaptureMouse;
}

//----------------------------------------
// ImguiPlatformBackend::ProcessMouseMove
//
bool ImguiPlatformBackend::ProcessMouseMove(ivec2 const& mousePos, core::T_KeyModifierFlags const modifiers)
{
	UpdateKeyModifiers(modifiers);

	m_LastValidMousePos = math::vecCast<float>(mousePos);

	ImGuiIO& io = ImGui::GetIO();
	io.AddMousePosEvent(m_LastValidMousePos.x, m_LastValidMousePos.y);

	return io.WantCaptureMouse;
}

//----------------------------------------------
// ImguiPlatformBackend::ProcessMouseWheelDelta
//
bool ImguiPlatformBackend::ProcessMouseWheelDelta(ivec2 const& mouseWheel, core::T_KeyModifierFlags const modifiers)
{
	UpdateKeyModifiers(modifiers);

	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseWheelEvent(static_cast<float>(mouseWheel.x), static_cast<float>(mouseWheel.y));

	return io.WantCaptureMouse;
}

//----------------------------------------------
// ImguiPlatformBackend::ProcessMouseEnterLeave
//
bool ImguiPlatformBackend::ProcessMouseEnterLeave(bool const entered, core::T_KeyModifierFlags const modifiers)
{
	UpdateKeyModifiers(modifiers);

	ImGuiIO& io = ImGui::GetIO();
	if (entered)
	{
		io.AddMousePosEvent(m_LastValidMousePos.x, m_LastValidMousePos.y);
	}
	else
	{
		m_LastValidMousePos = io.MousePos;
		io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
	}

	return false;
}

//----------------------------------------
// ImguiPlatformBackend::ProcessTextInput
//
bool ImguiPlatformBackend::ProcessTextInput(core::E_Character const character)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(static_cast<uint32>(character)); 

	return io.WantTextInput;
}


//-----------------------------------------
// ImguiPlatformBackend::UpdateMouseCursor
//
void ImguiPlatformBackend::UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();

	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) // #todo: also check if the cursor is disabled once engine supports that
	{
		return;
	}

	core::E_CursorShape const cursor = ImguiUtil::GetCursorShape(ImGui::GetMouseCursor());
	if (cursor != m_LastCursorShape)
	{
		m_CursorShapeManager->SetCursorShape(cursor);
		m_LastCursorShape = cursor;
	}
}

//------------------------------------------
// ImguiPlatformBackend::UpdateKeyModifiers
//
void ImguiPlatformBackend::UpdateKeyModifiers(core::T_KeyModifierFlags const modifiers) const
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(ImGuiKey_ModCtrl, (modifiers & core::KM_Control) != 0);
	io.AddKeyEvent(ImGuiKey_ModShift, (modifiers & core::KM_Shift) != 0);
	io.AddKeyEvent(ImGuiKey_ModAlt, (modifiers & core::KM_Alt) != 0);
	io.AddKeyEvent(ImGuiKey_ModSuper, (modifiers & core::KM_Super) != 0);
}


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)

