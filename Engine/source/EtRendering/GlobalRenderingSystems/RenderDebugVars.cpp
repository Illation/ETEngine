#include "stdafx.h"
#include "RenderDebugVars.h"


#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)

#include <EtCore/Util/DebugCommandController.h>


namespace et {
namespace render {


//---------------------------------
// DebugVars::Init
//
void DebugVars::Init()
{
	core::dbg::CommandController& cmdController = core::dbg::CommandController::Instance();

	cmdController.AddCommand(core::dbg::Command("rndr_toggle_freeze_frustum", "Freeze camera frustums in their current position"),
		core::dbg::T_CommandFn([this](core::dbg::Command const& command, std::string const& parameters)
			{
				ET_UNUSED(command);
				ET_UNUSED(parameters);
				m_IsFrustumFrozen = !m_IsFrustumFrozen;
				return core::dbg::E_CommandRes::Success;
			}));

	cmdController.AddCommand(core::dbg::Command("rndr_scene_render_mode", "[Shaded / Wireframe] Set the mode for scene rendering"),
		core::dbg::T_CommandFn([this](core::dbg::Command const& command, std::string const& parameters)
			{
				ET_UNUSED(command);

				if (core::reflection::EnumFromStringSafe(parameters, m_RenderMode))
				{
					m_OverrideRenderMode = true;
					return core::dbg::E_CommandRes::Success;
				}

				ET_TRACE_W(ET_CTX_RENDER, "Invalid render mode, valid modes:");
				for (uint8 modeIdx = 0u; modeIdx < static_cast<uint8>(E_RenderMode::COUNT); ++modeIdx)
				{
					ET_TRACE_I(ET_CTX_RENDER, "\t%s", core::reflection::EnumString(static_cast<E_RenderMode>(modeIdx)).c_str());
				}

				return core::dbg::E_CommandRes::IncorrecParameters;
			}));

	cmdController.AddCommand(core::dbg::Command("rndr_reset_scene_render_mode", "reset the render mode"),
		core::dbg::T_CommandFn([this](core::dbg::Command const& command, std::string const& parameters)
			{
				ET_UNUSED(command);
				ET_UNUSED(parameters);
				m_OverrideRenderMode = false;
				return core::dbg::E_CommandRes::Success;
			}));

	cmdController.AddCommand(core::dbg::Command("rndr_toggle_hide_atmospheres", "Hide atmospheres"),
		core::dbg::T_CommandFn([this](core::dbg::Command const& command, std::string const& parameters)
			{
				ET_UNUSED(command);
				ET_UNUSED(parameters);
				m_HideAtmospheres = !m_HideAtmospheres;
				return core::dbg::E_CommandRes::Success;
			}));
}

//---------------------------------
// DebugVars::OverrideMode
//
void DebugVars::OverrideMode(E_RenderMode& renderMode) const
{
	if (m_OverrideRenderMode)
	{
		renderMode = m_RenderMode;
	}
}


} // namespace render
} // namespace et

#endif // ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)

