#include "stdafx.h"
#include "DebugConsole.h"

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include <EtCore/Util/DebugCommandController.h>


namespace et {
namespace rt {


//===============
// Debug Console
//===============


// static
std::string const DebugConsole::s_CommandString(" > ");


//---------------------
// DebugConsole::c-tor
//
// Add console control commands
//
DebugConsole::DebugConsole()
{
	core::dbg::CommandController& cmdController = core::dbg::CommandController::Instance();

	cmdController.AddCommand(core::dbg::Command("clear_console", "Clear the console"), core::dbg::T_CommandFn(
		[this](core::dbg::Command const& command, std::string const& parameters) 
		{
			ET_UNUSED(command);
			ET_UNUSED(parameters);
			ClearLog();
			return core::dbg::E_CommandRes::Success;
		}));

	cmdController.AddCommand(core::dbg::Command("list_commands", "List all possible commands"), core::dbg::T_CommandFn(
		[this](core::dbg::Command const& command, std::string const& parameters) 
		{
			ET_UNUSED(command);
			ET_UNUSED(parameters);
			ListCommands();
			return core::dbg::E_CommandRes::Success;
		}));

	cmdController.AddCommand(core::dbg::Command("toggle_debug_info", "Show information about the amount of draw calls"), core::dbg::T_CommandFn(
		[this](core::dbg::Command const& command, std::string const& parameters)
		{
			ET_UNUSED(command);
			ET_UNUSED(parameters);
			m_DrawDebugInfo = !m_DrawDebugInfo;
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
		char const* wordEnd = callbackData->Buf + callbackData->CursorPos;
		char const* wordStart = wordEnd;
		while (wordStart > callbackData->Buf)
		{
			char const c = wordStart[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
			{
				break;
			}

			wordStart--;
		}

		int32 const length = static_cast<int32>(wordEnd - wordStart);


		core::dbg::CommandController const& comController = core::dbg::CommandController::Instance();

		// list of candidates
		std::string const currentWord(wordStart, length);
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
			callbackData->DeleteChars(static_cast<int32>(wordStart - callbackData->Buf), length);
			callbackData->InsertChars(callbackData->CursorPos, &candidates[0].m_Name[0]);
			callbackData->InsertChars(callbackData->CursorPos, "");
		}
		else
		{
			// Multiple matches. Complete as much as we can..
			// So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
			int32 matchLength = length;
			for (;;)
			{
				int c = 0;
				bool allCandidatesMatch = true;
				for (size_t i = 0u; i < candidates.size() && allCandidatesMatch; i++)
				{
					if (i == 0)
					{
						c = toupper(candidates[i].m_Name[matchLength]);
					}
					else if (c == 0 || c != toupper(candidates[i].m_Name[matchLength]))
					{
						allCandidatesMatch = false;
					}
				}

				if (!allCandidatesMatch)
				{
					break;
				}

				matchLength++;
			}

			if (matchLength > 0)
			{
				char* const firstChar = &candidates[0].m_Name[0];
				callbackData->DeleteChars(static_cast<int32>(wordStart - callbackData->Buf), length);
				callbackData->InsertChars(callbackData->CursorPos, firstChar, firstChar + matchLength);
			}

			// list possible matches
			m_LogLines.emplace_back("Possible matches");
			for (core::dbg::CommandIdInfo const& info : candidates)
			{
				m_LogLines.emplace_back(FS("\t- %s", info.m_Name.c_str()));
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
				m_PreHistoryText = m_InputText;
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

		if (prevHistoryPos != m_HistoryPos)
		{
			char const* const historyStr = (m_HistoryPos >= 0) ? m_History[m_HistoryPos].m_Text.c_str() : m_PreHistoryText.c_str();
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
	m_LogLines.emplace_back("All commands");
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
	// log the command
	m_LogLines.emplace_back(s_CommandString + m_InputText, vec4(1.f, 0.5f, 1.f, 1.f));

	// update command history
	core::HashString const historyId(m_InputText.c_str());
	auto const foundIt = std::find_if(m_History.cbegin(), m_History.cend(), [historyId](HistoryLine const& line)
		{
			return (line.m_Id == historyId);
		});

	if (foundIt != m_History.cend()) // if the input text is already in the history, we just move that history to the end
	{
		m_History.erase(foundIt);
	}

	m_History.emplace_back(m_InputText, historyId);
	m_PreHistoryText.clear();
	m_HistoryPos = -1;

	// execute the command
	core::dbg::CommandController const& comController = core::dbg::CommandController::Instance();

	core::HashString commandId;
	core::dbg::E_CommandRes const res = comController.ExecuteCommand(m_InputText, commandId);

	// print the result
	switch (res)
	{
	case core::dbg::E_CommandRes::Success:
		m_LogLines.emplace_back("Success", vec4(0.5f, 1.f, 0.5f, 1.f));
		break;

	case core::dbg::E_CommandRes::Error:
		m_LogLines.emplace_back("Error", vec4(1.f, 0.5f, 0.5f, 1.f));
		break;

	case core::dbg::E_CommandRes::IncorrecParameters:
		m_LogLines.emplace_back("Incorrect parameters", vec4(1.f, 1.f, 0.5f, 1.f));
		break;

	case core::dbg::E_CommandRes::NotFound:
		m_LogLines.emplace_back("Command not found", vec4(1.f, 1.f, 0.5f, 1.f));
		break;

	case core::dbg::E_CommandRes::PrintHelp:
	{
		core::dbg::Command const* const cmd = comController.GetCommand(commandId);
		ET_ASSERT(cmd != nullptr);
		m_LogLines.emplace_back(cmd->m_Usage);
		break;
	}

	default:
		ET_ASSERT(false, "unhandled command result");
	}

	// reset 
	m_InputText.clear();
	m_ScrollToBottom = true;
}


} // namespace rt
} // namespace et

#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)

