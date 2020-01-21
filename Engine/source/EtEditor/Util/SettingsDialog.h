#pragma once

#include <gtkmm/dialog.h>
#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>

#include <gtkmm/fontbutton.h>
#include <gtkmm/comboboxtext.h>


namespace et {
namespace edit {


//---------------------------------
// SettingsDialog
//
// Dialog that allows us to change the applications settings
//
class SettingsDialog : public Gtk::Dialog
{
public:
	SettingsDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder);

	static SettingsDialog* create(Gtk::Window& parent);

protected:

	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
	Glib::RefPtr<Gio::Settings> m_Settings;
};


} // namespace edit
} // namespace et
