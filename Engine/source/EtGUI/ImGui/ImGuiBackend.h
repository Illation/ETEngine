#pragma once
#include <imgui/imgui.h>

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include "ImguiPlatformBackend.h"
#include "ImguiRenderBackend.h"

#include <EtCore/UpdateCycle/Tickable.h>

#include <EtRHI/GraphicsContext/ViewportEvents.h>


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
		Ptr<rhi::Viewport> const viewport);
	void Deinit();

	// tickable interface
	//--------------------
private:
	void OnTick() override; 

	// functionality
	//---------------
	void Render();

	// utility
	//---------
	void SetupStyle();


	// Data
	///////

	bool m_HasFrame = false;

	ImguiPlatformBackend m_PlatformBackend;
	ImguiRenderBackend m_RenderBackend;

	rhi::T_ViewportEventCallbackId m_VPCallbackId = rhi::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
