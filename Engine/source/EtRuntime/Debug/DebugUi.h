#pragma once
#include <imgui/imgui.h>

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include "DebugConsole.h"

#include <EtCore/UpdateCycle/Tickable.h>


namespace et {
namespace rt {


//---------
// DebugUi
//
class DebugUi final : public core::I_Tickable
{
	// construct destruct
	//--------------------
public:
	DebugUi(); 

	// interface
	//-----------
protected:
	void OnTick() override;

	// utility
	//---------
private:
	void DrawDebugInfo();

	// Data
	///////

	DebugConsole m_DebugConsole;
	bool m_DebugConsoleEnabled = false;
	E_KbdKey m_ConsoleToggleKey = E_KbdKey::GraveAccent;
};


} // namespace rt
} // namespace et

#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
