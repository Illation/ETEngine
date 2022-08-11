#pragma once
#include <EtRendering/Extensions/RenderEvents.h>

#include <EtGUI/Rendering/GuiRenderer.h>

#include <EtFramework/GUI/GuiExtension.h>


namespace et {
namespace fw {


class RmlGlobal;


//---------------------------------
// SceneRendererGUI
//
// Renderer that renders objects in the GUI extension
//
class SceneRendererGUI final
{
public:
	// construct destruct
	//--------------------
	SceneRendererGUI() = default;
	~SceneRendererGUI();

	void Init(Ptr<render::T_RenderEventDispatcher> const eventDispatcher);
	void Deinit();

	// functionality
	//---------------
private:
	void DrawInWorld(rhi::T_FbLoc const targetFb, rhi::E_PolygonMode const polyMode, GuiExtension& guiExt, core::slot_map<mat4> const& nodes);
	void DrawOverlay(rhi::T_FbLoc const targetFb, rhi::E_PolygonMode const polyMode, GuiExtension& guiExt);


	// Data
	///////

	bool m_IsInitialized = false;

	gui::GuiRenderer m_GuiRenderer;

	// render event hooks
	Ptr<render::T_RenderEventDispatcher> m_EventDispatcher;
	render::T_RenderEventCallbackId m_WorldCallbackId = render::T_RenderEventDispatcher::INVALID_ID;
	render::T_RenderEventCallbackId m_OverlayCallbackId = render::T_RenderEventDispatcher::INVALID_ID;
};


} // namespace fw
} // namespace et
