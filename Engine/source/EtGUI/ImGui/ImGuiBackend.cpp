#include "stdafx.h"
#include "ImGuiBackend.h"


#if ET_CT_IS_ENABLED(ET_CT_IMGUI)


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
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	SetupStyle();

	// Setup Platform/Renderer backends
	m_PlatformBackend.Init(cursorManager, clipboardController, viewport);
	m_RenderBackend.Init();

	m_VPCallbackId = viewport->GetEventDispatcher().Register(render::E_ViewportEvent::VP_PostFlush | render::E_ViewportEvent::VP_NewRenderer,
		render::T_ViewportEventCallback([this](render::T_ViewportEventFlags const flags, render::ViewportEventData const* const data) -> void
			{
				ET_UNUSED(data); // can access targetFb here
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


//--------------------------
// ImGuiBackend::SetupStyle
//
void ImGuiBackend::SetupStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	// colors	
	//--------
	ImGui::StyleColorsClassic(&style);
	style.Colors[ImGuiCol_Text] = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.44f, 0.32f, 0.52f, 0.77f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.49f, 0.31f, 0.62f, 0.44f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.48f, 0.48f, 0.80f, 0.65f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.38f, 0.27f, 0.54f, 0.83f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.32f, 0.63f, 0.87f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.55f, 0.40f, 0.80f, 0.19f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.59f, 0.40f, 0.80f, 0.30f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.91f, 0.81f, 0.97f, 0.98f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.54f, 0.45f, 0.70f, 0.62f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.50f, 0.40f, 0.90f, 0.45f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.49f, 0.34f, 0.68f, 0.79f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.40f, 0.40f, 0.73f, 0.84f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.28f, 0.28f, 0.57f, 0.82f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.35f, 0.35f, 0.65f, 0.84f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.45f, 1.00f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	// shape
	//-------

	// padding
	style.WindowPadding = ImVec2(8.f, 8.f);
	style.FramePadding = ImVec2(4.f, 4.f);
	style.CellPadding = ImVec2(4.f, 4.f);
	style.ItemSpacing = ImVec2(8.f, 4.f);
	style.ItemInnerSpacing = ImVec2(4.f, 4.f);
	style.TouchExtraPadding = ImVec2(0.f, 0.f);
	style.IndentSpacing = 21.f;
	style.ScrollbarSize = 14.f;
	style.GrabMinSize = 12.f;

	// borders
	style.WindowBorderSize = 1.f;
	style.ChildBorderSize = 1.f;
	style.PopupBorderSize = 1.f;
	style.FrameBorderSize = 0.f;
	style.TabBorderSize = 0.f;

	// rounding
	style.WindowRounding = 8.f;
	style.ChildRounding = 0.f;
	style.FrameRounding = 6.f;
	style.PopupRounding = 0.f;
	style.ScrollbarRounding = 9.f;
	style.GrabRounding = 6.f;
	style.LogSliderDeadzone = 4.f;
	style.TabRounding = 4.f;

	// alignment
	style.WindowTitleAlign = ImVec2(0.f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.f, 0.f);

	// safe area padding
	style.DisplaySafeAreaPadding = ImVec2(3.f, 3.f);
}


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
