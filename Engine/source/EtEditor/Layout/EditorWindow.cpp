#include "stdafx.h"
#include "EditorWindow.h"

#include "EditorBase.h"

#include <gtkmm/object.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gtkmm/settings.h>
#include <gtkmm/frame.h>

#include <EtEditor/EditorApp.h>
#include <EtEditor/Util/GtkUtil.h>
#include <EtEditor/Util/EditorTickOrder.h>


namespace et {
namespace edit {


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

	m_RefBuilder->get_widget("editorSwitcher", m_EditorStack);
	ET_ASSERT(m_EditorStack != nullptr);

	auto realizeCallback = [this](Cairo::RefPtr<Cairo::Context> const&)
	{
		// Destroy all editors
		for (T_EditorFramePair& pair : m_Editors)
		{
			pair.first->OnAllocationAvailable();
		}

		return false;
	};
	signal_draw().connect(realizeCallback, true);

	//show all the widgets
	show_all_children();

	// Display the application menu in the application, not in the desktop environment.
	Glib::RefPtr<Gtk::Settings> gtk_settings = Gtk::Settings::get_default();
	if (gtk_settings)
	{
		gtk_settings->property_gtk_shell_shows_app_menu() = false;
	}
	set_show_menubar(true);

	auto keyPressedCallback = [this](GdkEventKey* evnt) -> bool
	{
		EditorBase* const activeEditor = GetVisibleEditor();
		if (activeEditor == nullptr)
		{
			return false;
		}

		return activeEditor->OnKeyEvent(true, evnt);
	};
	signal_key_press_event().connect(keyPressedCallback, true);

	// keyboard release
	auto keyReleasedCallback = [this](GdkEventKey* evnt) -> bool
	{
		EditorBase* const activeEditor = GetVisibleEditor();
		if (activeEditor == nullptr)
		{
			return false;
		}

		return activeEditor->OnKeyEvent(false, evnt);
	};
	signal_key_release_event().connect(keyReleasedCallback, true);

	// Set the window icon.
	set_icon(Gdk::Pixbuf::create_from_resource("/com/leah-lindner/editor/ui/icons/menu.png"));
}

//---------------------------------
// EditorAppWindow::d-tor
//
EditorAppWindow::~EditorAppWindow()
{
	// Destroy all editors
	for (T_EditorFramePair& pair : m_Editors)
	{
		SafeDelete(pair.first); // frame widget and children will automatically be deleted as it is managed by GTK
	}
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
	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/editorWindow.ui");

	// get the toplevel element
	EditorAppWindow* window = nullptr;
	refBuilder->get_widget_derived("app_window", window);
	if (!window)
	{
		throw std::runtime_error("No 'app_window' object in editorWindow.ui");
	}

	window->Init(editorApp);

	return window;
}

//---------------------------------
// EditorAppWindow::Init
//
// Set the editor app so we can display components of the editor
//
void EditorAppWindow::Init(EditorApp *const editorApp)
{
	m_EditorApp = editorApp;
}

//---------------------------------
// EditorAppWindow::AddEditor
//
// Adds an editor to the stack and initializes it. We should ensure here there is only one scene editor
//
void EditorAppWindow::AddEditor(EditorBase* const editor)
{
	ET_ASSERT(editor != nullptr);
	ET_ASSERT(std::find_if(m_Editors.begin(), m_Editors.end(), [editor](T_EditorFramePair const& editorFrame)
		{
			return editorFrame.first == editor;
		}) == m_Editors.cend(), "Editor already added");

	Gtk::Frame* childFrame = Gtk::make_managed<Gtk::Frame>();
	childFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

	m_EditorStack->add(*childFrame, editor->GetName(), editor->GetName());

	show_all_children();

	editor->Init(childFrame);

	show_all_children();

	m_Editors.emplace_back(editor, childFrame);
}

//---------------------------------
// EditorAppWindow::RemoveEditor
//
// Closes an editor and its tools
//
void EditorAppWindow::RemoveEditor(EditorBase* const editor)
{
	ET_ASSERT(editor != nullptr);
	auto editorIt = std::find_if(m_Editors.begin(), m_Editors.end(), [editor](T_EditorFramePair const& editorFrame)
		{
			return editorFrame.first == editor;
		});

	ET_ASSERT(editorIt != m_Editors.cend());

	SafeDelete(editorIt->first);

	m_EditorStack->remove(*(editorIt->second));
	SafeDelete(editorIt->second); // since the frame was managed we need to delete it manually now that it no longer has a parent

	// we don't swap because order might matter in the future, and the list will probably never have enough pointers to impact performance
	m_Editors.erase(editorIt); 
}

//---------------------------------
// EditorAppWindow::SaveLayout
//
// Retrieve the editor that is currently visible in the stack
//
void EditorAppWindow::SaveLayout()
{
	EditorBase* const activeEditor = GetVisibleEditor();
	ET_ASSERT(activeEditor != nullptr);

	activeEditor->SaveLayout();
}

//---------------------------------
// EditorAppWindow::GetVisibleEditor
//
// Retrieve the editor that is currently visible in the stack
//
EditorBase* EditorAppWindow::GetVisibleEditor() const
{
	// get the visible frame
	Gtk::Widget* childWidget = m_EditorStack->get_visible_child();
	if (childWidget == nullptr)
	{
		return nullptr;
	}

	// find an editor bound to that frame
	auto const foundPairIt = std::find_if(m_Editors.cbegin(), m_Editors.cend(), [childWidget](T_EditorFramePair const& childPair)
		{
			return childPair.second == childWidget;
		});
	if (foundPairIt == m_Editors.cend())
	{
		return nullptr;
	}

	return foundPairIt->first;
}


} // namespace edit
} // namespace et
