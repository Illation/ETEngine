#pragma once
#include "TextRenderer.h"
#include "SpriteRenderer.h"

#include <EtGUI/GuiExtension.h>

#include <EtRendering/Extensions/RenderEvents.h>


namespace et {
namespace gui {


//---------------------------------
// GuiRenderer
//
// Renderer that renders objects in the GUI extension
//
class GuiRenderer final
{
public:
	// construct destruct
	//--------------------
	GuiRenderer() = default;
	~GuiRenderer();

	void Init(Ptr<render::T_RenderEventDispatcher> const eventDispatcher);
	void Deinit();

	// functionality
	//---------------
	void DrawInWorld(render::T_FbLoc const targetFb, GuiExtension const& guiExt, core::slot_map<mat4> const& nodes);
	void DrawOverlay(render::T_FbLoc const targetFb);

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

	// basic renderers
	TextRenderer m_TextRenderer;
	SpriteRenderer m_SpriteRenderer;
};


} // namespace gui
} // namespace et
