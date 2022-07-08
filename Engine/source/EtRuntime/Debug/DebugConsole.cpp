#include "stdafx.h"
#include "DebugConsole.h"

#if ET_IMGUI_ENABLED


namespace et {
namespace rt {


//===============
// Debug Console
//===============


//--------------------
// DebugConsole::Draw
//
void DebugConsole::Draw()
{
	ImGuiWindowFlags const windowFlags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoMove;

	ImGuiViewport const* const viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowPos(viewport->WorkSize);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0.f));
	if (ImGui::Begin("Debug Console", nullptr, windowFlags))
	{
		ImGui::PopStyleVar(2);

		DrawMenu();
		DrawConsole();

		ImGui::PopStyleVar();
	}
	else
	{
		ImGui::PopStyleVar(3);
	}

	ImGui::End();
}

//------------------------
// DebugConsole::DrawMenu
//
void DebugConsole::DrawMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::MenuItem("Show Debug Info", "", &m_DrawDebugInfo);
			ImGui::MenuItem("Show ImGui Demo Window", "", &m_DrawDemoWindow);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Clear Log"))
			{
				ClearLog();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

//---------------------------
// DebugConsole::DrawConsole
//
void DebugConsole::DrawConsole()
{
	ImGuiViewport const* const viewport = ImGui::GetMainViewport();

	vec2 workSize = viewport->WorkSize;
	workSize.y *= 0.5f;

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(workSize);
	if (ImGui::Begin("Internal Console", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
	{
		// Log Area
		//----------
		float const offset = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("Log Area", vec2(0.f, -offset), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear"))
			{
				ClearLog();
			}

			ImGui::EndPopup();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		for (LogLine const& line : m_LogLines)
		{
			ImGui::TextColored(line.m_Color, line.m_Text.c_str());
		}

		if (m_ScrollToBottom)
		{
			ImGui::SetScrollHereY(1.f);
			m_ScrollToBottom = false;
		}

		ImGui::PopStyleVar();
		ImGui::EndChild();

		ImGui::Separator();

		// Input Field
		//-------------
		static std::string const s_CommandString(" > ");
		ImGui::Text(s_CommandString.c_str());
		ImGui::SameLine();

		bool reclaimFocus = false;
		ImGui::PushItemWidth(-1.f);
		if (ImGui::InputText("Input", &m_InputText[0], m_InputText.capacity(), 
			ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackHistory,
			[](ImGuiInputTextCallbackData* const callbackData) -> int32
			{
				return static_cast<DebugConsole*>(callbackData->UserData)->TextCallback(callbackData);
			}, this))
		{
			m_LogLines.emplace_back(s_CommandString + m_InputText);
			m_History.push_back(m_InputText); // #todo: keep history unique
			m_HistoryPos = -1;

			m_InputText.clear();

			m_ScrollToBottom = true;
			reclaimFocus = true;
		}

		ImGui::PopItemWidth();

		// auto focus
		ImGui::SetItemDefaultFocus();
		if (reclaimFocus)
		{
			ImGui::SetKeyboardFocusHere(-1);
		}
	}

	ImGui::End();
}

//----------------------------
// DebugConsole::TextCallback
//
int32 DebugConsole::TextCallback(ImGuiInputTextCallbackData* const callbackData)
{
	switch (callbackData->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackResize:
		m_InputText.resize(callbackData->BufTextLen);
		callbackData->Buf = &m_InputText[0];
		break;

	case ImGuiInputTextFlags_CallbackHistory:
	{
		int32 const prevHistoryPos = m_HistoryPos;
		if (callbackData->EventKey == ImGuiKey_UpArrow)
		{
			if (m_HistoryPos == -1)
			{
				m_HistoryPos = static_cast<int32>(m_History.size()) - 1;
			}
			else if (m_HistoryPos > 0)
			{
				m_HistoryPos--;
			}
		}
		else if (callbackData->EventKey == ImGuiKey_DownArrow)
		{
			if (m_HistoryPos != -1)
			{
				if (++m_HistoryPos >= static_cast<int32>(m_History.size()))
				{
					m_HistoryPos = -1;
				}
			}
		}

		// #todo: remember the input text when no history item is selected
		if (prevHistoryPos != m_HistoryPos)
		{
			char const* const historyStr = (m_HistoryPos >= 0) ? m_History[m_HistoryPos].c_str() : "";
			callbackData->DeleteChars(0, callbackData->BufTextLen);
			callbackData->InsertChars(0, historyStr);
		}

		break;
	}
	}

	return 0;
}

//----------------------------
// DebugConsole::ClearLog
//
void DebugConsole::ClearLog()
{
	m_LogLines.clear();
}


} // namespace rt
} // namespace et

#endif // ET_IMGUI_ENABLED

