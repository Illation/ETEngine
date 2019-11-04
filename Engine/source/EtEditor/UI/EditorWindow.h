#pragma once

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>
#include <gtkmm/stack.h>
#include <giomm/settings.h>


//forward declarations
namespace Gtk {
	class Frame;
}

class Gio::File;

class EditorApp;
class EditorBase;


//---------------------------------
// EditorAppWindow
//
// Main window for this project
//
class EditorAppWindow final : public Gtk::ApplicationWindow
{
	// definitions
	//--------------
public:
	typedef std::pair<EditorBase*, Gtk::Frame*> T_EditorFramePair;

	// construct destruct
	//--------------------
	EditorAppWindow(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder);
	virtual ~EditorAppWindow();

	static EditorAppWindow* create(EditorApp *const editorApp);
	void Init(EditorApp *const editorApp);

	// functionality
	//---------------
	void AddEditor(EditorBase* const editor); // takes ownership
	void RemoveEditor(EditorBase* const editor);

	void SaveLayout();

private:

	EditorBase* GetVisibleEditor() const;

	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
	Glib::RefPtr<Gio::Settings> m_Settings;

	EditorApp* m_EditorApp = nullptr;

	Gtk::Stack* m_EditorStack = nullptr;

	std::vector<T_EditorFramePair> m_Editors;
};