#pragma once

#include "EditorTool.h"

#include <Engine/GraphicsHelper/Viewport.h>

#include <gtkmm/frame.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>
#include "EditorTool.h"


class SceneEditor;


//--------------------
// SceneViewport
//
// Space to draw the scene in
//
class SceneViewport final : public I_EditorTool
{
public:
	// construct destruct
	//--------------------
	SceneViewport(); 
	~SceneViewport();

	// Editor tool interface
	//-----------------------
	void Init(I_Editor* const editor, Gtk::Frame* parent) override;

	// functionality
	//---------------
	void ShowSplashScreen();
	void InitRenderingSystems();

	// Data
	///////

private:

	SceneEditor* m_Editor;

	std::unique_ptr<Viewport> m_Viewport;

	bool m_IsNavigating = false;

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
};
