#include "stdafx.h"
#include "ResourceChooserDialog.h"


namespace et {
namespace edit {


//=========================
// Resource Chooser Dialog
//=========================


//static
//---------------------------------
// ResourceChooserDialog::create
//
ResourceChooserDialog* ResourceChooserDialog::create()
{
	// Load the Builder file and instantiate its widgets.
	Glib::RefPtr<Gtk::Builder> const refBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/resourceChooser.ui");

	ResourceChooserDialog* dialog = nullptr;
	refBuilder->get_widget_derived("resource_dialog", dialog);
	ET_ASSERT(dialog != nullptr, "No 'resource_dialog' object in resourceChooser.ui");

	return dialog;
}


//----------------------------------------------
// ResourceChooserDialog::ResourceChooserDialog
//
// Settings Dialog default constructor
//
ResourceChooserDialog::ResourceChooserDialog(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder)
	: Gtk::Dialog(cobject)
	, m_RefBuilder(refBuilder)
{
	// area for displaying asset widgets
	m_RefBuilder->get_widget("top_level", m_TopLevel);
	ET_ASSERT(m_TopLevel != nullptr);

	m_View.Init();

	m_TopLevel->pack_start(*(m_View.GetAttachment()), true, true, 0);
}


} // namespace edit
} // namespace et
