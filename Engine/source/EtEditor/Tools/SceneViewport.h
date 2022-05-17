#pragma once
#include <gtkmm/frame.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>
#include <gdk/gdk.h>

#include <EtRendering/GraphicsContext/Viewport.h>
#include <EtRendering/Extensions/OutlineRenderer.h>

#include <EtGUI/Rendering/GuiRenderer.h>
#include <EtGUI/Content/SpriteFont.h>

#include <EtFramework/SceneGraph/SceneEvents.h>

#include <EtEditor/SceneEditor/SceneEditorListener.h>
#include <EtEditor/Layout/EditorTool.h>


// forward
namespace et { namespace render {
	class ShadedSceneRenderer;
} }


namespace et {
namespace edit {


class SceneEditor;


//--------------------
// SceneViewport
//
// Space to draw the scene in
//
class SceneViewport final : public I_EditorTool, public I_SceneEditorListener
{
public:
	// construct destruct
	//--------------------
	SceneViewport(); 
	~SceneViewport();

	// Editor tool interface
	//-----------------------
	void Init(EditorBase* const editor, Gtk::Frame* const parent) override;
	void OnDeinit() override;
	bool IsToolbarTopPref() const override { return false; }

	// scene editor listener interface
	//---------------------------------
	void OnShown();
	void OnSceneSet();
	void OnEditorTick();

	// functionality
	//----------------
	bool OnKeyEvent(bool const pressed, GdkEventKey* const evnt);

private:
	// utility
	//----------
	void InitCamera();

	// Data
	///////

	SceneEditor* m_Editor;

	fw::T_EntityId m_Camera = fw::INVALID_ENTITY_ID;
	std::unique_ptr<render::Viewport> m_Viewport;
	render::I_RenderArea* m_RenderArea = nullptr;

	render::ShadedSceneRenderer* m_SceneRenderer = nullptr;
	gui::GuiRenderer m_GuiRenderer;
	render::OutlineRenderer m_OutlineRenderer;

	fw::T_SceneEventCallbackId m_SceneInitCallback = fw::T_SceneEventDispatcher::INVALID_ID;

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

	AssetPtr<gui::SpriteFont> m_DebugFont;
	bool m_DrawDebugInfo = true;

	bool m_IsInitialized = false;
};


} // namespace edit
} // namespace et
