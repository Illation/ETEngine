#include <imgui/imgui.h>

#if ET_IMGUI_ENABLED


namespace et {
namespace rt {


//--------------
// DebugConsole
//
// runtime app console that can execute debug commands (on non shipping builds)
//
class DebugConsole 
{
	// definitions
	//-------------
	struct LogLine final
	{
		LogLine(std::string const& text, vec4 const& col = vec4(1.f)) : m_Text(text), m_Color(col) {}

		std::string m_Text;
		vec4 m_Color;
	};

	// construct destruct
	//--------------------
public:
	DebugConsole();
	virtual ~DebugConsole() = default;

	// functionality
	//---------------
	void Draw();

	// accessors
	//-----------
	bool DrawDebugInfoEnabled() const { return m_DrawDebugInfo; }
	bool DrawImguiDemoEnabled() const { return m_DrawDemoWindow; }

	// utility
	//---------
private:
	void DrawMenu();
	void DrawConsole();

	int32 TextCallback(ImGuiInputTextCallbackData* const callbackData);

	void ClearLog();
	void ListCommands();
	void ExecuteCommand();


	// Data
	///////

	bool m_DrawDebugInfo = false;
	bool m_DrawDemoWindow = false;

	std::vector<LogLine> m_LogLines;
	bool m_ScrollToBottom = false;

	std::string m_InputText;

	int32 m_HistoryPos = -1;
	std::vector<std::string> m_History;
};


} // namespace rt
} // namespace et

#endif // ET_IMGUI_ENABLED
