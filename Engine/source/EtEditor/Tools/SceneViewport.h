#pragma once
#include <gtkmm/frame.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>
#include <gdk/gdk.h>

#include <EtCore/Input/RawInputProvider.h>

#include <EtRHI/GraphicsContext/Viewport.h>

#include <EtRendering/Extensions/OutlineRenderer.h>

#include <EtGUI/Fonts/SdfFont.h>

#include <EtFramework/GUI/SceneRendererGUI.h>
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
	void OnEditorTick() {}

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
	UniquePtr<rhi::Viewport> m_Viewport;
	rhi::I_RenderArea* m_RenderArea = nullptr;

	render::ShadedSceneRenderer* m_SceneRenderer = nullptr;
	fw::SceneRendererGUI m_SceneGuiRenderer;
	render::OutlineRenderer m_OutlineRenderer;

	core::RawInputProvider m_InputProvider;

	fw::T_SceneEventCallbackId m_SceneInitCallback = fw::T_SceneEventDispatcher::INVALID_ID;

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

	AssetPtr<gui::SdfFont> m_DebugFont;
	bool m_DrawDebugInfo = true;

	bool m_IsInitialized = false;
};


} // namespace edit
} // namespace et
