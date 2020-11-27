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
	static ResourceChooserDialog* create(std::vector<rttr::type> const& allowedTypes);

	// construct destruct
	//--------------------
	ResourceChooserDialog(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder, std::vector<rttr::type> const& allowedTypes);

	// accessors
	//-----------
	std::vector<core::I_Asset*> const& GetSelectedAssets() const { return m_SelectedAssets; }

	// utility
	//--------
private:
	void OnSelectedAssetsChanged();


	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

	Gtk::Box* m_TopLevel = nullptr; // the view gets attached to this
	Gtk::Button* m_SelectButton = nullptr;
	
	ResourceView m_View;

	std::vector<core::I_Asset*> m_SelectedAssets;
};


} // namespace edit
} // namespace et
