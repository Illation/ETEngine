#include "stdafx.h"
#include "DebugUi.h"

#if ET_IMGUI_ENABLED

#include <EtBuild/EngineVersion.h>


namespace et {
namespace rt {


//==========
// Debug UI
//==========


//-------------------------
// DebugUi::OnTick
//
void DebugUi::OnTick()
{
	core::InputManager* const input = core::InputManager::GetInstance();

	if (input->GetKeyState(E_KbdKey::H) == E_KeyState::Pressed)
	{
		m_DrawDebugInfo = !m_DrawDebugInfo;
	}

	DrawDebugInfo();

	// Demo Window
	if (input->GetKeyState(E_KbdKey::J) == E_KeyState::Pressed)
	{
		m_DrawDemoWindow = !m_DrawDemoWindow;
	}

	if (m_DrawDemoWindow)
	{
		ImGui::ShowDemoWindow(&m_DrawDemoWindow);
	}
}

//-------------------------
// DebugUi::DrawDebugInfo
//
void DebugUi::DrawDebugInfo()
{
	ImGuiWindowFlags const overlayFlags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove;

	float const pad = 10.0f;
	ImGuiViewport const* const viewport = ImGui::GetMainViewport();
	// Use work area to avoid menu-bar/task-bar, if any!

	// Version info
	//--------------

	ImGui::SetNextWindowPos(vec2(viewport->WorkPos) + vec2(viewport->WorkSize) - pad, ImGuiCond_Always, vec2(1.f, 1.f));
	ImGui::SetNextWindowBgAlpha(0.15f); 
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	if (ImGui::Begin("Version Info:", nullptr, overlayFlags))
	{
		ImGui::TextColored(vec4(0.5f, 9.f, 0.5f, 0.6f), 
			"%sE.T.Engine - %s", 
			m_DrawDebugInfo ? "" : FS("%i FPS - ", PERFORMANCE->GetRegularFPS()).c_str(), 
			et::build::Version::s_Name.c_str());
	}

	ImGui::End();
	ImGui::PopStyleVar();

	// Debug info
	//------------

	if (m_DrawDebugInfo)
	{
		ImGui::SetNextWindowPos(vec2(viewport->WorkPos) + pad, ImGuiCond_Always, ImVec2(0.f, 0.f));
		ImGui::SetNextWindowBgAlpha(0.35f);

		if (ImGui::Begin("Debug Info:", &m_DrawDebugInfo, overlayFlags))
		{
			ImGui::Text("FPS: %i", PERFORMANCE->GetRegularFPS());
			ImGui::Text("Frame ms: %f", PERFORMANCE->GetFrameMS());
			ImGui::Separator();
			ImGui::Text("Draw Calls: %u", PERFORMANCE->m_PrevDrawCalls);
		}

		ImGui::End();
	}
}


} // namespace rt
} // namespace et

#endif // ET_IMGUI_ENABLED
