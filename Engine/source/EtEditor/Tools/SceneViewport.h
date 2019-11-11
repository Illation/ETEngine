#pragma once
#include <Engine/GraphicsContext/Viewport.h>

#include <gtkmm/frame.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>
#include <gdk/gdk.h>

#include <EtEditor/SceneEditor/SceneEditorListener.h>
#include <EtEditor/Rendering/EditorCamera.h>
#include <EtEditor/Layout/EditorTool.h>


// forward
class SceneEditor;
class SceneRenderer;


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

	EditorCamera m_Camera;
	std::unique_ptr<Viewport> m_Viewport;
	I_RenderArea* m_RenderArea = nullptr;
	SceneRenderer* m_SceneRenderer = nullptr;

	T_SceneEventCallbackId m_SceneInitCallback = SceneEventDispatcher::s_InvalidCallbackId;

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

	bool m_IsInitialized = false;
};