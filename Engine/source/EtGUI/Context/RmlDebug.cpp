#include "stdafx.h"
#include "RmlDebug.h"

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include "TickOrder.h"

#include <EtCore/Util/DebugCommandController.h>


namespace et {
namespace gui {


//============
// RML Debug
//============


//------------------------
// RmlDebug::RmlDebug
//
RmlDebug::RmlDebug() 
	: core::I_Tickable(static_cast<uint32>(E_TickOrder::TICK_GuiDebug))
{
	core::dbg::CommandController& cmdController = core::dbg::CommandController::Instance();

	cmdController.AddCommand(core::dbg::Command("gui_debug", "Show debugging options for the GUI system"), core::dbg::T_CommandFn(
		[this](core::dbg::Command const& command, std::string const& parameters)
		{
			ET_UNUSED(command);
			ET_UNUSED(parameters);
			m_IsVisible = !m_IsVisible;
			return core::dbg::E_CommandRes::Success;
		}));
}

//------------------------
// RmlDebug::OnTick
//
void RmlDebug::OnTick()
{
	if (m_IsVisible)
	{
		if (ImGui::Begin("GUI Debug", &m_IsVisible))
		{
			
		}

		ImGui::End();
	}
}


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
