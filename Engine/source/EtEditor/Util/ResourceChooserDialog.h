#pragma once

#include <giomm/settings.h>

#include <glibmm/refptr.h>

#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeselection.h>


namespace et { namespace core {
	class Directory;
	class I_Asset;
} }


namespace et {
namespace edit {


//---------------------------------
// ResourceChooserDialog
//
class AssetWidget final
{
public:
	AssetWidget(core::I_Asset* const asset);

	Gtk::Frame* GetAttachment() const { return m_Attachment; }

	// Data
	///////

private:
	core::I_Asset* m_Asset = nullptr; // not using assetptr since we don't want the widget to enforce loading the asset

	Gtk::Frame* m_Attachment = nullptr;
	bool m_HasPreview = false;
};

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

	static ResourceChooserDialog* create();

	// construct destruct
	//--------------------
	ResourceChooserDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder);

private:

	// events
	//--------
	void ResourceGroupToggled();
	void OnDirectorySelectionChanged();

	// utility
	//---------
	void RebuildDirectoryTree();
	void RebuildAssetList();

	// Data
	/////////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

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
};


} // namespace edit
} // namespace et
