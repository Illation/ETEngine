#include "stdafx.h"
#include "DebugConsole.h"

#if ET_IMGUI_ENABLED

#include <EtCore/Util/DebugCommandController.h>


namespace et {
namespace rt {


//===============
// Debug Console
//===============


//---------------------
// DebugConsole::c-tpr
//
DebugConsole::DebugConsole()
{
	core::dbg::CommandController::Instance().AddCommand(core::dbg::Command("clear", "Clear the console"), core::dbg::T_CommandFn(
		[this](core::dbg::Command const& command, std::string const& parameters) -> core::dbg::E_CommandRes
		{
			UNUSED(command);
			UNUSED(parameters);
			ClearLog();
			return core::dbg::E_CommandRes::Success;
		}));

	core::dbg::CommandController::Instance().AddCommand(core::dbg::Command("list_commands", "List all possible commands"), core::dbg::T_CommandFn(
		[this](core::dbg::Command const& command, std::string const& parameters) -> core::dbg::E_CommandRes
		{
			UNUSED(command);
			UNUSED(parameters);
			ListCommands();
			return core::dbg::E_CommandRes::Success;
		}));
}

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

		ImGuiInputTextFlags const inputFlags = 
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_CallbackResize |
			ImGuiInputTextFlags_CallbackHistory |
			ImGuiInputTextFlags_CallbackCompletion;

		bool reclaimFocus = false;
		ImGui::PushItemWidth(-1.f);
		if (ImGui::InputText("Input", &m_InputText[0], m_InputText.capacity(), inputFlags, [](ImGuiInputTextCallbackData* const callbackData) -> int32
			{
				return static_cast<DebugConsole*>(callbackData->UserData)->TextCallback(callbackData);
			}, this))
		{
			m_LogLines.emplace_back(s_CommandString + m_InputText);
			ExecuteCommand();

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

	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// Locate beginning of current word
		const char* word_end = callbackData->Buf + callbackData->CursorPos;
		const char* word_start = word_end;
		while (word_start > callbackData->Buf)
		{
			const char c = word_start[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
				break;
			word_start--;
		}


		core::dbg::CommandController const& comController = core::dbg::CommandController::Instance();

		// list of candidates
		std::string const currentWord(word_start, static_cast<int>(word_end - word_start));
		std::vector<core::dbg::CommandIdInfo> candidates;
		for (core::dbg::CommandIdInfo const& info : comController.GetCommandInfoSet())
		{
			if (info.m_Name.rfind(currentWord, 0) == 0) // starts with
			{ 
				candidates.push_back(info);
			}
		}

		if (candidates.empty())
		{
			m_LogLines.emplace_back(FS("No match for %s", currentWord), vec4(1.f, 1.f, 0.5f, 1.f));
		}
		else if (candidates.size() == 1u)
		{
			callbackData->DeleteChars((int)(word_start - callbackData->Buf), (int)(word_end - word_start));
			callbackData->InsertChars(callbackData->CursorPos, &candidates[0].m_Name[0]);
			callbackData->InsertChars(callbackData->CursorPos, " ");
		}
		else
		{
			m_LogLines.emplace_back("Possible matches");
			for (core::dbg::CommandIdInfo const& info : candidates)
			{
				m_LogLines.emplace_back(FS("\t%s", info.m_Name.c_str()));
			}
		}

		break;
	}

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

//----------------------------
// DebugConsole::ClearLog
//
void DebugConsole::ListCommands()
{
	core::dbg::CommandController const& comController = core::dbg::CommandController::Instance();
	for (core::dbg::CommandIdInfo const& info : comController.GetCommandInfoSet())
	{
		m_LogLines.emplace_back(FS("\t%s", info.m_Name.c_str()));
	}
}

//------------------------------
// DebugConsole::ExecuteCommand
//
void DebugConsole::ExecuteCommand()
{
	m_History.push_back(m_InputText); // #todo: keep history unique
	m_HistoryPos = -1;

	core::dbg::E_CommandRes const res = core::dbg::CommandController::Instance().ExecuteCommand(m_InputText);
	switch (res)
	{
	case core::dbg::E_CommandRes::Success:
		m_LogLines.emplace_back("Success", vec4(0.5f, 1.f, 0.5f, 1.f));
		break;

	case core::dbg::E_CommandRes::Error:
		m_LogLines.emplace_back("Error", vec4(1.f, 0.5f, 0.5f, 1.f));
		break;

	case core::dbg::E_CommandRes::IncorrecParameters:
		m_LogLines.emplace_back("Incorrect paramters", vec4(1.f, 1.f, 0.5f, 1.f));
		break;

	case core::dbg::E_CommandRes::NotFound:
		m_LogLines.emplace_back("Command not found", vec4(1.f, 1.f, 0.5f, 1.f));
		break;

	default:
		ET_ASSERT(false, "unhandled command result");
	}

	m_InputText.clear();

	m_ScrollToBottom = true;
}


} // namespace rt
} // namespace et

#endif // ET_IMGUI_ENABLED

