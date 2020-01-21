#include "stdafx.h"
#include "SettingsDialog.h"


namespace et {
namespace edit {


//====================
// Settings Dialog
//====================


//---------------------------------
// SettingsDialog::SettingsDialog
//
// Settings Dialog default constructor
//
SettingsDialog::SettingsDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder)
	: Gtk::Dialog(cobject)
	, m_RefBuilder(refBuilder)
	, m_Settings()
{
	m_Settings = Gio::Settings::create("com.leah-lindner.editor");
}


//static
//---------------------------------
// SettingsDialog::create
//
// Create a settings dialog from the generated source in prefs.ui
//
SettingsDialog* SettingsDialog::create(Gtk::Window& parent)
{
	// Load the Builder file and instantiate its widgets.
	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/prefs.ui");

	SettingsDialog* dialog = nullptr;
	refBuilder->get_widget_derived("prefs_dialog", dialog);
	if (!dialog)
	{
		throw std::runtime_error("No 'prefs_dialog' object in prefs.ui");
	}

	return dialog;
}


} // namespace edit
} // namespace et
