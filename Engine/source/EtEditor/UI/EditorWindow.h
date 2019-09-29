#pragma once

#include "Outliner.h"

#include <Engine/GraphicsHelper/Viewport.h>

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>
#include <gtkmm/stack.h>
#include <giomm/settings.h>


//forward declarations
class Gio::File;
class EditorApp;


//---------------------------------
// EditorAppWindow
//
// Main window for this project
//
class EditorAppWindow final : public Gtk::ApplicationWindow
{
public:
	EditorAppWindow(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder);
	virtual ~EditorAppWindow() = default;

	static EditorAppWindow* create(EditorApp *const editorApp);
	void SetEditorApp(EditorApp *const editorApp);

	void Init();

	static E_MouseButton GetButtonFromGtk(uint32 const buttonCode);
	static E_KbdKey GetKeyFromGtk(uint32 const keyCode);

private:

	std::unique_ptr<Viewport> CreateSceneViewport();
	std::unique_ptr<Outliner> CreateOutliner();

	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
	Glib::RefPtr<Gio::Settings> m_Settings;

	EditorApp* m_EditorApp = nullptr;

	bool m_IsNavigating = false;

	std::unique_ptr<Viewport> m_SceneViewport;
	std::unique_ptr<Outliner> m_Outliner;
};