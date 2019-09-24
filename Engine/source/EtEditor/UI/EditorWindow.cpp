#include "stdafx.h"
#include "EditorWindow.h"

#include <EtEditor/EditorApp.h>
#include <EtEditor/Rendering/GtkRenderArea.h>

#include <EtCore/Helper/InputManager.h>

#include <gtkmm/object.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gtkmm/settings.h>
#include <gtkmm/glarea.h>

#include <Engine/GraphicsHelper/SceneRenderer.h>


namespace Gtk
{
	template<class T, class... T_Args>
	auto make_managed(T_Args&&... args)
	{
		return manage(new T(std::forward<T_Args>(args)...));
	}
}


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

	// create a new viewport
	std::string viewName = "Scene View";
	std::unique_ptr<Viewport> viewport = std::move(CreateViewport(viewName));

	// Set the viewports renderer to the oscillator
	viewport->SetRenderer(SceneRenderer::GetInstance());

	// manage the pointer to the viewport
	m_Viewports.emplace_back(std::move(viewport));
}

//---------------------------------
// EditorAppWindow::Init
//
// After we have an openGl context and a resource manager we sho9uld show the splash screen
//
void EditorAppWindow::Init()
{
	ET_ASSERT(!m_Viewports.empty());
	//m_Viewports[0]->MakeCurrent();

	SceneRenderer::GetInstance()->InitWithSplashScreen();

	m_Viewports[0]->Redraw();
}

//---------------------------------
// EditorAppWindow::CreateViewport
//
// Create a viewport with an openGL area in it
//
std::unique_ptr<Viewport> EditorAppWindow::CreateViewport(std::string const& name)
{
	// get the stack
	Gtk::Stack* viewportStack;
	m_RefBuilder->get_widget("stack", viewportStack);
	ET_ASSERT(viewportStack != nullptr, "No 'stack' object in window.ui!");

	// add a GL area to the stack
	Gtk::GLArea* glArea = Gtk::make_managed<Gtk::GLArea>();
	glArea->set_auto_render(true);

	// create a viewport from the area
	std::unique_ptr<Viewport> viewport = std::make_unique<Viewport>(new GtkRenderArea(glArea));

	glArea->show();
	viewportStack->add(*glArea, name, name);

	return std::move(viewport);
}
