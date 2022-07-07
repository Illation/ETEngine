#pragma once
#include <imgui/imgui.h>


#if ET_IMGUI_ENABLED

#include <EtCore/Input/KeyCodes.h>
#include <EtCore/Util/CursorShapes.h>


namespace et {
namespace gui {

namespace ImguiUtil {


ImGuiMouseButton GetButton(E_MouseButton const button);
ImGuiKey GetKey(E_KbdKey const key);
core::E_CursorShape GetCursorShape(ImGuiMouseCursor const cursor);


} // namespace ImguiUtil

} // namespace gui
} // namespace et


#endif // ET_IMGUI_ENABLED
