#include "stdafx.h"
#include "ResourceView.h"

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>

#include <EtPipeline/Core/Content/FileResourceManager.h>


namespace et {
namespace edit {


//===============
// Resource View
//===============


//----------------------------------------------
// ResourceView::ResourceView
//
ResourceView::ResourceView()
{
	m_RefBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/resourceView.ui");
}

//----------------------------------------------
// ResourceView::Init
//
// Link everything together
//  - if allowed types is empty every asset type is selectable
//
void ResourceView::Init(std::vector<rttr::type> const& allowedTypes)
{
	// attachment to parent widget
	m_RefBuilder->get_widget("resource_viewer", m_Attachment);
	ET_ASSERT(m_Attachment != nullptr);

	// area for displaying asset widgets
	m_RefBuilder->get_widget("resource_view", m_FlowBox);
	ET_ASSERT(m_FlowBox != nullptr);

	m_RefBuilder->get_widget("asset_area", m_AssetArea);
	ET_ASSERT(m_AssetArea != nullptr);

	m_FlowBox->signal_selected_children_changed().connect(sigc::mem_fun(*this, &ResourceView::OnSelectedChildrenChanged));

	// filtering
	m_RefBuilder->get_widget("toolbar", m_Toolbar); // allows detatching the filter and search options and putting them in a toolbar instead
	ET_ASSERT(m_Toolbar != nullptr);

	Gtk::MenuButton* filterButton;
	m_RefBuilder->get_widget("filter", filterButton);
	ET_ASSERT(filterButton != nullptr);
	m_TypeFilter.RegisterListener(this); // no need to unregister ourselves because we own it
	m_TypeFilter.Init(filterButton, allowedTypes);

	m_RefBuilder->get_widget("search", m_SearchBar);
	ET_ASSERT(m_SearchBar != nullptr);

	m_SearchBar->signal_search_changed().connect(sigc::mem_fun(*this, &ResourceView::OnSearchChanged));

	// toggle view of engine or project resources
	m_RefBuilder->get_widget("engine_button", m_EngineButton);
	ET_ASSERT(m_EngineButton != nullptr);
	m_RefBuilder->get_widget("project_button", m_ProjectButton);
	ET_ASSERT(m_ProjectButton != nullptr);
	
	m_EngineButton->signal_toggled().connect(sigc::mem_fun(*this, &ResourceView::ResourceGroupToggled));
	m_ProjectButton->signal_toggled().connect(sigc::mem_fun(*this, &ResourceView::ResourceGroupToggled));
	if (m_ProjectSelected)
	{
		m_ProjectButton->set_active(true);
	}
	else
	{
		m_EngineButton->set_active(true);
	}

	// directory tree
	m_RefBuilder->get_widget("directory_tree", m_TreeView);
	ET_ASSERT(m_TreeView != nullptr);

	m_TreeModel = Gtk::TreeStore::create(m_Columns);

	m_TreeView->set_model(m_TreeModel);

	m_TreeSelection = m_TreeView->get_selection();
	m_TreeSelection->signal_changed().connect(sigc::mem_fun(*this, &ResourceView::OnDirectorySelectionChanged));

	m_TreeView->append_column("Directory", m_Columns.m_Name);

	RebuildDirectoryTree();
}

//-------------------------
// ResourceView::Rebuild
//
// Update which assets are shown
//
void ResourceView::Rebuild()
{
	std::string const selectedDir = m_SelectedDirectory;

	RebuildDirectoryTree(); // this triggers rebuild asset list

	Gtk::TreeModel::Children children = m_TreeModel->children();
	for (core::Directory const* const baseDir : m_BaseDirectories)
	{
		auto row = RecursiveGetDirectory(baseDir->GetName() + selectedDir, children);
		if (row)
		{
			m_TreeSelection->select(row);
			break;
		}
	}
}

//------------------------------------------------
// ResourceView::ResourceGroupToggled
//
void ResourceView::ResourceGroupToggled()
{
	bool changed = false;

	if (m_ProjectButton->get_active())
	{
		ET_ASSERT(!(m_EngineButton->get_active()));

		if (!m_ProjectSelected)
		{
			m_ProjectSelected = true;
			changed = true;
		}
	}
	else
	{
		ET_ASSERT(!(m_ProjectButton->get_active()));
		ET_ASSERT(m_EngineButton->get_active());

		if (m_ProjectSelected)
		{
			m_ProjectSelected = false;
			changed = true;
		}
	}

	if (changed && (m_TreeView != nullptr))
	{
		RebuildDirectoryTree();
	}
}

//----------------------------------------------------
// ResourceView::OnDirectorySelectionChanged
//
void ResourceView::OnDirectorySelectionChanged()
{
	m_SelectedBaseDir = nullptr;
	Gtk::TreeModel::iterator const& it = m_TreeSelection->get_selected(Glib::RefPtr<Gtk::TreeModel>::cast_static(m_TreeModel));
	if (it != m_TreeModel->children().end())
	{
		core::Directory* const dir = (*it)[m_Columns.m_Directory];
		if (dir != nullptr)
		{
			for (core::Directory* const baseDir : m_BaseDirectories)
			{
				if (dir->IsChildOf(baseDir))
				{
					m_SelectedDirectory = core::FileUtil::GetRelativePath(dir->GetName(), baseDir->GetName());
					m_SelectedBaseDir = baseDir;
					break;
				}
			}
		}
		else
		{
			m_SelectedDirectory.clear();
		}
	}
	else
	{
		m_SelectedDirectory.clear();
	}

	RebuildAssetList();
}

//-----------------------------------------
// ResourceView::OnSearchChanged
//
void ResourceView::OnSearchChanged()
{
	m_SearchTerm = std::string(m_SearchBar->get_text());

	RebuildAssetList();
}

//-------------------------------------------------
// ResourceView::OnAssetTypeFilterChanged
//
void ResourceView::OnAssetTypeFilterChanged()
{
	RebuildAssetList();
}

//-------------------------------------------------
// ResourceView::OnAssetTypeFilterChanged
//
void ResourceView::OnSelectedChildrenChanged()
{
	std::vector<Gtk::FlowBoxChild*> selectedChildren = m_FlowBox->get_selected_children();

	m_SelectedAssets.clear();
	for (Gtk::FlowBoxChild* child : selectedChildren)
	{
		int32 const childIdx = child->get_index();
		ET_ASSERT(childIdx < m_FilteredAssets.size());

		AssetWidget& assetWidget = m_FilteredAssets[childIdx];
		ET_ASSERT(assetWidget.GetAttachment() == child->get_child());

		m_SelectedAssets.push_back(&assetWidget);
	}

	m_SignalSelectionChanged.emit();
}

//------------------------------------------------
// ResourceView::RebuildDirectoryTree
//
void ResourceView::RebuildDirectoryTree()
{
	pl::FileResourceManager* const resourceMan = static_cast<pl::FileResourceManager*>(core::ResourceManager::Instance());
	ET_ASSERT(resourceMan != nullptr);

	m_BaseDirectories = m_ProjectSelected ? resourceMan->GetProjectDatabase().GetDirectories() : resourceMan->GetEngineDatabase().GetDirectories();

	m_TreeModel->clear();

	for (core::Directory* const dir : m_BaseDirectories)
	{
		Gtk::TreeModel::Row row = *(m_TreeModel->append());
		row[m_Columns.m_Name] = dir->GetNameOnly().substr(0, dir->GetNameOnly().size() - 1).c_str();
		row[m_Columns.m_Directory] = dir;

		auto recursiveAddChildren = [this](Gtk::TreeModel::Row& row, auto& fnRef) mutable -> void
		{
			core::Directory* const baseDir = row[m_Columns.m_Directory];
			ET_ASSERT(baseDir != nullptr);

			for (core::Entry* const entry : baseDir->GetChildren())
			{
				if (entry->GetType() == core::Entry::EntryType::ENTRY_DIRECTORY)
				{
					core::Directory* const childDir = static_cast<core::Directory*>(entry);

					Gtk::TreeModel::Row childRow = *(m_TreeModel->append(row.children()));
					childRow[m_Columns.m_Name] = childDir->GetNameOnly().substr(0, childDir->GetNameOnly().size() - 1).c_str();
					childRow[m_Columns.m_Directory] = childDir;

					fnRef(childRow, fnRef);
				}
			}
		};

		recursiveAddChildren(row, recursiveAddChildren);

		m_TreeSelection->select(row);
	}

	m_TreeView->show_all_children();
	m_TreeView->expand_all();
}

//------------------------------------------------
// ResourceView::RebuildDirectoryTree
//
void ResourceView::RebuildAssetList()
{
	m_FilteredAssets.clear();

	// clear flow box
	for (Gtk::Widget* const child : m_FlowBox->get_children())
	{
		m_FlowBox->remove(*child);
	}

	// if we only allow specific types of assets and no asset types are selected, there will be nothing to populate
	if (!m_TypeFilter.AreAllTypesAllowed() && m_TypeFilter.GetFilteredTypes().empty())
	{
		return;
	}

	// fetch assets
	pl::FileResourceManager* const resourceMan = static_cast<pl::FileResourceManager*>(core::ResourceManager::Instance());
	ET_ASSERT(resourceMan != nullptr);

	pl::EditorAssetDatabase& database = m_ProjectSelected ? resourceMan->GetProjectDatabase() : resourceMan->GetEngineDatabase();
	std::vector<pl::EditorAssetBase*> const filteredAssets = database.GetAssetsMatchingQuery(m_SelectedDirectory,
		m_TypeFilter.AreDirectoriesRecursive(), 
		m_SearchTerm, 
		m_TypeFilter.GetFilteredTypes(),
		m_SelectedBaseDir);

	// repopulate
	for (pl::EditorAssetBase* const asset : filteredAssets)
	{
		m_FilteredAssets.emplace_back(asset);
		AssetWidget& assetWidget = m_FilteredAssets.back();
		m_FlowBox->add(*(assetWidget.GetAttachment()));
	}

	m_FlowBox->show_all_children();
}

//------------------------------------------------
// ResourceView::RecursiveGetDirectory
//
Gtk::TreeModel::Row ResourceView::RecursiveGetDirectory(std::string const dir, Gtk::TreeModel::Children const& children) const 
{
	for (Gtk::TreeModel::Children::iterator it = children.begin(); it != children.end(); ++it)
	{
		Gtk::TreeModel::Row row = *it;

		core::Directory const* const rowDir = row[m_Columns.m_Directory];
		std::string rowDirPath(rowDir->GetName());
		core::FileUtil::RemoveExcessPathDelimiters(rowDirPath);
		if (rowDirPath == dir)
		{
			return row;
		}

		row = RecursiveGetDirectory(dir, it->children());
		if (row)
		{
			return row;
		}
	}

	return *(children.end());
}


} // namespace edit
} // namespace et

