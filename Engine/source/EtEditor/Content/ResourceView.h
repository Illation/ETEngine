#pragma once

#include <glibmm/refptr.h>

#include <gtkmm/builder.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/paned.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeselection.h>

#include "AssetWidget.h"
#include "AssetTypeFilter.h"


namespace et { namespace core {
	class Directory;
} }


namespace et {
namespace edit {


//---------------------------------
// ResourceView
//
// Widget that allows browsing for assets
//
class ResourceView : public AssetTypeFilter::I_Listener
{
public:

	// definitions
	//--------------------
	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
		{
			add(m_Name);
			add(m_Directory);
		}

		Gtk::TreeModelColumn<Glib::ustring> m_Name;
		Gtk::TreeModelColumn<core::Directory*> m_Directory;
	};

	// construct destruct
	//--------------------
	ResourceView();
	void Init();

	// accessors
	//-----------
	Gtk::Widget* GetAttachment() const { return m_Attachment; }

private:

	// events
	//--------
	void ResourceGroupToggled();
	void OnDirectorySelectionChanged();
	void OnSearchChanged();
	void OnAssetTypeFilterChanged() override;

	// utility
	//---------
	void RebuildDirectoryTree();
	void RebuildAssetList();


	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
	Gtk::Paned* m_Attachment = nullptr;

	// resource view
	Gtk::FlowBox* m_FlowBox = nullptr;

	// engine / project resources
	Gtk::RadioButton* m_EngineButton = nullptr;
	Gtk::RadioButton* m_ProjectButton = nullptr;

	// Directory view
	Gtk::TreeView* m_TreeView = nullptr;

	ModelColumns m_Columns;
	Glib::RefPtr<Gtk::TreeStore> m_TreeModel;
	Glib::RefPtr<Gtk::TreeSelection> m_TreeSelection;

	// results
	bool m_ProjectSelected = true;
	std::string m_SelectedDirectory;
	core::Directory* m_BaseDirectory = nullptr;
	std::vector<AssetWidget> m_FilteredAssets;

	// filter
	AssetTypeFilter m_TypeFilter;

	// search
	Gtk::SearchEntry* m_SearchBar = nullptr;
	std::string m_SearchTerm;
};


} // namespace edit
} // namespace et
