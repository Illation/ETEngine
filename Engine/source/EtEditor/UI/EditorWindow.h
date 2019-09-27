#pragma once

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

	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
	Glib::RefPtr<Gio::Settings> m_Settings;

	EditorApp* m_EditorApp = nullptr;

	std::unique_ptr<Viewport> m_SceneViewport;
};