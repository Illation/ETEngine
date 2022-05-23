#pragma once
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Types.h>

#include <EtCore/Input/KeyCodes.h>


namespace et {
namespace gui {

namespace RmlUtil {

	int32 GetRmlButtonIndex(E_MouseButton const button);
	Rml::Input::KeyIdentifier GetRmlKeyId(E_KbdKey const key);
	int32 GetRmlModifierFlags(core::T_KeyModifierFlags const mods);

} // namespace RmlUtil

} // namespace gui
} // namespace et

