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

	typedef sigc::signal<void> T_SignalSelectionChanged;

	// construct destruct
	//--------------------
	ResourceView();
	void Init(std::vector<rttr::type> const& allowedTypes);

	// accessors
	//-----------
	Gtk::Widget* GetAttachment() const { return m_Attachment; }
	std::string const& GetSelectedDirectory() const { return m_SelectedDirectory; }
	bool IsProjectSelected() const { return m_ProjectSelected; }
	std::vector<AssetWidget*> const& GetSelectedAssets() const { return m_SelectedAssets; }
	T_SignalSelectionChanged GetSelectionChangeSignal() const { return m_SignalSelectionChanged; }
	Gtk::Box* GetToolbar() const { return m_Toolbar; }
	Gtk::Widget* GetAssetArea() const { return m_AssetArea; }

	// functionality
	//---------------
	void Rebuild();

private:

	// events
	//--------
	void ResourceGroupToggled();
	void OnDirectorySelectionChanged();
	void OnSearchChanged();
	void OnAssetTypeFilterChanged() override;
	void OnSelectedChildrenChanged();

	// utility
	//---------
	void RebuildDirectoryTree();
	void RebuildAssetList();
	Gtk::TreeModel::Row RecursiveGetDirectory(std::string const dir, Gtk::TreeModel::Children const& children) const;


	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
	Gtk::Paned* m_Attachment = nullptr;

	// resource view
	Gtk::FlowBox* m_FlowBox = nullptr;
	Gtk::Widget* m_AssetArea = nullptr;

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
	core::Directory* m_SelectedBaseDir = nullptr;
	std::vector<core::Directory*> m_BaseDirectories;
	std::vector<AssetWidget> m_FilteredAssets;
	std::vector<AssetWidget*> m_SelectedAssets;

	// toolbar
	Gtk::Box* m_Toolbar = nullptr;

	// filter
	AssetTypeFilter m_TypeFilter;

	// search
	Gtk::SearchEntry* m_SearchBar = nullptr;
	std::string m_SearchTerm;

	T_SignalSelectionChanged m_SignalSelectionChanged;
};


} // namespace edit
} // namespace et
