#include "stdafx.h"
#include "ResourceChooserDialog.h"

namespace et {
namespace edit {


//=========================
// Resource Chooser Dialog
//=========================


//----------------------------------------------
// ResourceChooserDialog::ResourceChooserDialog
//
// Settings Dialog default constructor
//
ResourceChooserDialog::ResourceChooserDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder)
	: Gtk::Dialog(cobject)
	, m_RefBuilder(refBuilder)
{
}


//static
//---------------------------------
// ResourceChooserDialog::create
//
ResourceChooserDialog* ResourceChooserDialog::create(Gtk::Window& parent)
{
	// Load the Builder file and instantiate its widgets.
	Glib::RefPtr<Gtk::Builder> const refBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/resourceChooser.ui");

	ResourceChooserDialog* dialog = nullptr;
	refBuilder->get_widget_derived("prefs_dialog", dialog);
	ET_ASSERT(dialog != nullptr, "No 'prefs_dialog' object in resourceChooser.ui");

	return dialog;
}


} // namespace edit
} // namespace et
