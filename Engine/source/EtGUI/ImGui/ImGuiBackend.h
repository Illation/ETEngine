#pragma once
#include <imgui/imgui.h>

#ifndef IMGUI_DISABLE

#include "ImguiPlatformBackend.h"
#include "ImguiRenderBackend.h"

#include <EtCore/UpdateCycle/Tickable.h>

#include <EtRendering/GraphicsContext/ViewportEvents.h>


namespace et {
namespace gui {


//---------------
// ImGuiBackend
//
// Responsible for handling events and drawing the imgui to the GPU
//
class ImGuiBackend final : public core::I_Tickable
{
	// construct destruct
	//--------------------
public:
	ImGuiBackend(uint32 const priority) : core::I_Tickable(priority) {}

	void Init(Ptr<core::I_CursorShapeManager> const cursorManager,
		Ptr<core::I_ClipboardController> const clipboardController,
		Ptr<render::Viewport> const viewport);
	void Deinit();

	// tickable interface
	//--------------------
private:
	void OnTick() override; 
	void Render();


	// Data
	///////

	bool m_HasFrame = false;

	ImguiPlatformBackend m_PlatformBackend;
	ImguiRenderBackend m_RenderBackend;

	render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace gui
} // namespace et


#endif // ndef IMGUI_DISABLE
