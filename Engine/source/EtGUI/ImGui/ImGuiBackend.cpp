#include "stdafx.h"
#include "ImGuiBackend.h"


#ifndef IMGUI_DISABLE


namespace et {
namespace gui {


//===============
// ImGui Backend
//===============


//--------------------
// ImGuiBackend::Init
//
void ImGuiBackend::Init(Ptr<core::I_CursorShapeManager> const cursorManager,
	Ptr<core::I_ClipboardController> const clipboardController,
	Ptr<render::Viewport> const viewport)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	m_PlatformBackend.Init(cursorManager, clipboardController, viewport);
	m_RenderBackend.Init();

	m_VPCallbackId = viewport->GetEventDispatcher().Register(render::E_ViewportEvent::VP_PostFlush | render::E_ViewportEvent::VP_NewRenderer,
		render::T_ViewportEventCallback([this](render::T_ViewportEventFlags const flags, render::ViewportEventData const* const data) -> void
			{
				UNUSED(data); // can access targetFb here
				if (flags & render::E_ViewportEvent::VP_NewRenderer)
				{
					if (!m_HasFrame)
					{
						OnTick();
					}
				}
				else
				{
					Render();
				}
			}));
}

//----------------------
// ImGuiBackend::Deinit
//
void ImGuiBackend::Deinit()
{
	m_PlatformBackend.GetViewport()->GetEventDispatcher().Unregister(m_VPCallbackId);

	m_RenderBackend.Deinit();
	m_PlatformBackend.Deinit();
	ImGui::DestroyContext();
}

//----------------------
// ImGuiBackend::OnTick
//
void ImGuiBackend::OnTick()
{
	m_PlatformBackend.Update();
	m_RenderBackend.Update();
	ImGui::NewFrame();
	m_HasFrame = true;
}

//----------------------
// ImGuiBackend::Render
//
void ImGuiBackend::Render()
{
	if (!m_HasFrame)
	{
		return;
	}

	m_HasFrame = false;
	ImGui::Render();
	m_RenderBackend.Render(ImGui::GetDrawData());
}


} // namespace gui
} // namespace et


#endif // ndef IMGUI_DISABLE
