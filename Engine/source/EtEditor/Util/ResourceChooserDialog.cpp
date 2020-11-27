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
// if allowed types is empty every asset type is selectable
//
ResourceChooserDialog* ResourceChooserDialog::create(std::vector<rttr::type> const& allowedTypes)
{
	// Load the Builder file and instantiate its widgets.
	Glib::RefPtr<Gtk::Builder> const refBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/resourceChooser.ui");

	ResourceChooserDialog* dialog = nullptr;
	refBuilder->get_widget_derived("resource_dialog", dialog, allowedTypes);
	ET_ASSERT(dialog != nullptr, "No 'resource_dialog' object in resourceChooser.ui");

	return dialog;
}


//----------------------------------------------
// ResourceChooserDialog::ResourceChooserDialog
//
// Settings Dialog default constructor
//
ResourceChooserDialog::ResourceChooserDialog(BaseObjectType* cobject, 
	Glib::RefPtr<Gtk::Builder> const& refBuilder, 
	std::vector<rttr::type> const& allowedTypes
)
	: Gtk::Dialog(cobject)
	, m_RefBuilder(refBuilder)
{
	// area for displaying asset widgets
	m_RefBuilder->get_widget("top_level", m_TopLevel);
	ET_ASSERT(m_TopLevel != nullptr);

	m_SelectButton = add_button("Select", Gtk::ResponseType::RESPONSE_ACCEPT);
	m_SelectButton->set_sensitive(!m_SelectedAssets.empty());

	m_View.Init(allowedTypes);
	m_View.GetSelectionChangeSignal().connect(sigc::mem_fun(*this, &ResourceChooserDialog::OnSelectedAssetsChanged));

	m_TopLevel->pack_start(*(m_View.GetAttachment()), true, true, 0);
}

//------------------------------------------------
// ResourceChooserDialog::OnSelectedAssetsChanged
//
void ResourceChooserDialog::OnSelectedAssetsChanged()
{
	std::vector<AssetWidget*> const& selectedAssets = m_View.GetSelectedAssets();

	m_SelectedAssets.clear();
	for (AssetWidget* const selected : selectedAssets)
	{
		m_SelectedAssets.push_back(selected->GetAsset());
	}

	m_SelectButton->set_sensitive(!m_SelectedAssets.empty());
}


} // namespace edit
} // namespace et
