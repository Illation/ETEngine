#pragma once

#include "EditorTool.h"

#include <Engine/GraphicsContext/Viewport.h>

#include <gtkmm/frame.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>

#include <EtEditor/SceneEditorListener.h>


// forward
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
	void Init(EditorBase* const editor, Gtk::Frame* parent) override;

	// scene editor listener interface
	//---------------------------------
	void OnShown();
	void OnSceneSet();

	// Data
	///////

private:

	SceneEditor* m_Editor;

	std::unique_ptr<Viewport> m_Viewport;

	bool m_IsNavigating = false;

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
};
