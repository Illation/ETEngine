#pragma once
#include "ImGui.h"


#if ET_IMGUI_ENABLED


namespace et {
namespace gui {

namespace ImguiUtil {


ImGuiMouseButton GetButton(E_MouseButton const button);
ImGuiKey GetKey(E_KbdKey const key);


// Converting et::math types to Rml counterparts
ImVec2 ToImgui(vec2 const& rhs);

// Converting et::math types from Rml counterparts
vec2 ToEtm(ImVec2 const& rhs);


} // namespace ImguiUtil

} // namespace gui
} // namespace et


#endif // ET_IMGUI_ENABLED
