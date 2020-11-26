#pragma once

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>

#include <EtEditor/Content/ResourceView.h>


namespace et {
namespace edit {


//---------------------------------
// ResourceChooserDialog
//
// Dialog that allows the user to select a resource
//
class ResourceChooserDialog : public Gtk::Dialog
{
public:

	// definitions
	//--------------------
	static ResourceChooserDialog* create();

	// construct destruct
	//--------------------
	ResourceChooserDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder);                                                       

private:

	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
	Gtk::Box* m_TopLevel = nullptr; // the view gets attached to this
	
	ResourceView m_View;
};


} // namespace edit
} // namespace et
