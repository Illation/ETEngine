#pragma once
#include "ImGui.h"

#if ET_IMGUI_ENABLED

#include "ImguiPlatformBackend.h"
#include "ImguiRenderBackend.h"


namespace et {
namespace gui {


//---------------
// ImGuiBackend
//
// Responsible for handling events and drawing the imgui to the GPU
//
class ImGuiBackend final
{
	// construct destruct
	//--------------------
public:
	static void Init(ImguiPlatformBackend& platformBackend, ImguiRenderBackend& renderBackend);
	static void Deinit();

	// functionality
	//---------------
	static void NewFrame();
	static void Render();
};


} // namespace gui
} // namespace et


#endif // ET_IMGUI_ENABLED
