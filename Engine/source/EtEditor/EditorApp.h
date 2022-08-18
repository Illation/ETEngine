#pragma once

#include <EtCore/UpdateCycle/DefaultTickTriggerer.h>

#include <list>
#include <memory>

#include <gtkmm/application.h>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include <EtEditor/Rendering/GtkRenderWindow.h>


namespace et {
namespace edit {


// forward declarations
class EditorAppWindow;

//---------------------------------
// EditorApp
//
// Application for the engine editor
//
class EditorApp final : public Gtk::Application, public core::I_DefaultTickTriggerer
{
protected:
	EditorApp();
	virtual ~EditorApp();

public:
	static Glib::RefPtr<EditorApp> create();

private:
	EditorAppWindow* CreateMainWindow();
	void OnHideWindow(Gtk::Window* window);

protected:
	// Override default gtkmm application signal handlers:
	void on_startup() override;
	void on_activate() override;

private:
	void InitializeUtilities();

	// Runtime
	bool OnTick();
	void OnActionPreferences();
	void OnSaveEditorLayout();
	void OnActionQuit();

private:
	// Data
	////////
	EditorAppWindow* m_AppWindow = nullptr;
	GtkRenderWindow m_RenderWindow;
};


} // namespace edit
} // namespace et
