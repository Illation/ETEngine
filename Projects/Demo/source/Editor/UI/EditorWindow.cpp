#include "stdafx.h"
#include "EditorWindow.h"

#include <EditorApp.h>

#include <EtCore/Helper/InputManager.h>

#include <gtkmm/object.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gtkmm/settings.h>


//===========================
// Editor Application Window
//===========================


//---------------------------------
// EditorAppWindow::c-tor
//
EditorAppWindow::EditorAppWindow(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder)
	: Gtk::ApplicationWindow(cobject)
	, m_RefBuilder(refBuilder)
	, m_Settings()
{
	m_Settings = Gio::Settings::create("com.leah-lindner.editor");

	// listen for keyboard input
	// on press
	auto keyPressedCallback = [](GdkEventKey* evnt) -> bool
	{
		InputManager::GetInstance()->OnKeyPressed(static_cast<E_KbdKey>(evnt->keyval));
		return false;
	};
	signal_key_press_event().connect(keyPressedCallback, false);

	// on release
	auto keyReleasedCallback = [](GdkEventKey* evnt) -> bool
	{
		InputManager::GetInstance()->OnKeyReleased(static_cast<E_KbdKey>(evnt->keyval));
		return false;
	};
	signal_key_release_event().connect(keyReleasedCallback, false);

	//show all the widgets
	show_all_children();

	// Display the application menu in the application, not in the desktop environment.
	Glib::RefPtr<Gtk::Settings> gtk_settings = Gtk::Settings::get_default();
	if (gtk_settings)
	{
		gtk_settings->property_gtk_shell_shows_app_menu() = false;
	}
	set_show_menubar(true);

	// Set the window icon.
	set_icon(Gdk::Pixbuf::create_from_resource("/com/leah-lindner/editor/ui/icons/menu.png"));
}

//static
//---------------------------------
// EditorAppWindow::create
//
// Create a editor app window from the generated source in window.ui
//
EditorAppWindow* EditorAppWindow::create(EditorApp *const editorApp)
{
	// Load the Builder file and instantiate its widgets.
	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/window.ui");

	// get the toplevel element
	EditorAppWindow* window = nullptr;
	refBuilder->get_widget_derived("app_window", window);
	if (!window)
	{
		throw std::runtime_error("No 'app_window' object in window.ui");
	}

	window->SetEditorApp(editorApp);

	return window;
}

//---------------------------------
// EditorAppWindow::SetEditorApp
//
// Set the editor app so we can display components of the editor
//
void EditorAppWindow::SetEditorApp(EditorApp *const editorApp)
{
	m_EditorApp = editorApp;
}
