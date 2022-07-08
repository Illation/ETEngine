#pragma once
#include <imgui/imgui.h>

#if ET_IMGUI_ENABLED

#include "DebugConsole.h"

#include <EtCore/UpdateCycle/Tickable.h>

#include <EtFramework/Config/TickOrder.h>


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
	// we'll just reuse the framework tick, it doesn't really matter when we receive the tick as long as it's within the tick order
	DebugUi() : core::I_Tickable(static_cast<uint32>(fw::E_TickOrder::TICK_Framework)) {} 

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

#endif // ET_IMGUI_ENABLED
