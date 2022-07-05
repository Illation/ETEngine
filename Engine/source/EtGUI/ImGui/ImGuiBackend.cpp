#include "stdafx.h"
#include "ImGuiBackend.h"


#if ET_IMGUI_ENABLED


namespace et {
namespace gui {


//===============
// ImGui Backend
//===============


//--------------------
// ImGuiBackend::Init
//
void ImGuiBackend::Init(ImguiPlatformBackend& platformBackend, ImguiRenderBackend& renderBackend)
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
	platformBackend.Init();
	renderBackend.Init();
}

//----------------------
// ImGuiBackend::Deinit
//
void ImGuiBackend::Deinit()
{
	ImguiRenderBackend::AccessFromIO()->Deinit();
	ImguiPlatformBackend::AccessFromIO()->Deinit();
	ImGui::DestroyContext();
}

//------------------------
// ImGuiBackend::NewFrame
//
void ImGuiBackend::NewFrame()
{
	ImguiPlatformBackend::AccessFromIO()->Update();
	ImguiRenderBackend::AccessFromIO()->Update();
	ImGui::NewFrame();
}

//----------------------
// ImGuiBackend::Render
//
void ImGuiBackend::Render()
{
	ImGui::Render();
	ImguiRenderBackend::AccessFromIO()->Render(ImGui::GetDrawData());
}


} // namespace gui
} // namespace et


#endif // ET_IMGUI_ENABLED
