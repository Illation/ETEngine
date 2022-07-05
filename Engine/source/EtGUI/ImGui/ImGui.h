#pragma once


// include this file instead of including imgui directly
////////////////////////////////////////////////////////


#ifdef ET_SHIPPING
#	define ET_IMGUI_ENABLED false
#else
#	define ET_IMGUI_ENABLED true
#endif // ET_SHIPPING



#if ET_IMGUI_ENABLED
#	include <imgui/imgui.h>
#endif


