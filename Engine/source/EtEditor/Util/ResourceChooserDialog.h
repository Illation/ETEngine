#pragma once

#include <gtkmm/dialog.h>
#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>

#include <gtkmm/fontbutton.h>


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
	ResourceChooserDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder);

	static ResourceChooserDialog* create();

protected:

	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
};


} // namespace edit
} // namespace et
