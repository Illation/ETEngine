#pragma once

#include <EtCore/Util/InputManager.h>


namespace Gtk {


//---------------------------------
// make_managed
//
// Function that the GTKmm book suggests using but doesn't actually exist in the codebase
//
template<class T, class... T_Args>
auto make_managed(T_Args&&... args)
{
	return manage(new T(std::forward<T_Args>(args)...));
}


} // namespace Gtk


namespace et {
namespace edit {

namespace GtkUtil {

	E_MouseButton GetButtonFromGtk(uint32 const buttonCode);
	E_KbdKey GetKeyFromGtk(uint32 const keyCode);

} // namepace GtkUtil

} // namespace edit
} // namespace et

