#pragma once
#include "TextRenderer.h"
#include "SpriteRenderer.h"
#include "GuiRenderer.h"

#include <EtGUI/GuiExtension.h>

#include <EtRendering/Extensions/RenderEvents.h>


namespace et {
namespace gui {


class RmlGlobal;


//---------------------------------
// GuiRenderer
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
	void DrawInWorld(render::T_FbLoc const targetFb, GuiExtension& guiExt, core::slot_map<mat4> const& nodes);
	void DrawOverlay(render::T_FbLoc const targetFb, GuiExtension& guiExt, core::slot_map<mat4> const& nodes);

	// accessors
	//-----------
	TextRenderer& GetTextRenderer() { return m_TextRenderer; }
	SpriteRenderer& GetSpriteRenderer() { return m_SpriteRenderer; }

	// Data
	///////

private:
	bool m_IsInitialized = false;

	// render event hooks
	Ptr<render::T_RenderEventDispatcher> m_EventDispatcher;
	render::T_RenderEventCallbackId m_WorldCallbackId = render::T_RenderEventDispatcher::INVALID_ID;
	render::T_RenderEventCallbackId m_OverlayCallbackId = render::T_RenderEventDispatcher::INVALID_ID;

	// renderers
	//-----------
	TextRenderer m_TextRenderer;
	SpriteRenderer m_SpriteRenderer;
	GuiRenderer m_GuiRenderer;
};


} // namespace gui
} // namespace et
