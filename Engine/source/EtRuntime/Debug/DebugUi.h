#pragma once
#include <imgui/imgui.h>

#if ET_IMGUI_ENABLED

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

	bool m_DrawDebugInfo = false;
	bool m_DrawDemoWindow = false;
};


} // namespace rt
} // namespace et

#endif // ET_IMGUI_ENABLED
