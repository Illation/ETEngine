#pragma once
#include <RmlUi/Core/Input.h>

#include <EtCore/Input/KeyCodes.h>


namespace et {
namespace gui {

namespace RmlUtil {

	int32 GetRmlButtonIndex(E_MouseButton const button);
	Rml::Input::KeyIdentifier GetRmlKeyId(E_KbdKey const key);

} // namepace RmlUtil

} // namespace gui
} // namespace et

