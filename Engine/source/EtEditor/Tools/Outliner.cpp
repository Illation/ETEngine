#include "stdafx.h"
#include "Outliner.h"

#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/box.h>
#include <gtkmm/searchentry.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>

#include <EtEditor/SceneEditor/SceneEditor.h>
#include <EtEditor/Util/GtkUtil.h>


namespace et {
namespace edit {


//==========================
// Outliner
//==========================


//--------------------
// Outliner::c-tor
//
Outliner::Outliner()
	: I_EditorTool()
{
	m_RefBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/outliner.ui");
}

//--------------------
// Outliner::d-tor
//
Outliner::~Outliner()
{
	m_SceneSelection->UnregisterListener(this);
}

//--------------------
// Outliner::Init
//
// Tool initialization implementation
//
void Outliner::Init(EditorBase* const editor, Gtk::Frame* parent)
{
	m_SceneSelection = &(static_cast<SceneEditor*>(editor)->GetSceneSelection());

	// get the toplevel element
	Gtk::Box* box = nullptr;
	m_RefBuilder->get_widget("outliner", box);
	ET_ASSERT(box != nullptr);

	m_RefBuilder->get_widget("outlinerView", m_TreeView);
	ET_ASSERT(m_TreeView != nullptr);

	m_TreeModel = Gtk::TreeStore::create(m_Columns);

	m_TreeView->set_model(m_TreeModel);

	m_TreeSelection = m_TreeView->get_selection();
	m_TreeSelection->set_mode(Gtk::SELECTION_MULTIPLE);
	m_TreeSelection->signal_changed().connect(sigc::mem_fun(*this, &Outliner::OnSelectionChanged));

	//All the items to be reordered with drag-and-drop:
	m_TreeView->set_reorderable();

	m_TreeView->append_column("Name", m_Columns.m_Name);
	m_TreeView->append_column("ID", m_Columns.m_Id);

	m_TreeView->signal_row_activated().connect(sigc::mem_fun(*this, &Outliner::OnTreeViewRowActivated));

	m_SceneSelection->RegisterListener(this);

	if (fw::UnifiedScene::Instance().GetSceneId() != 0u)
	{
		RefillTreeView();
		for (fw::T_EntityId const entity : m_SceneSelection->GetSelectedEntities())
		{
			OnEntitySelectionChanged(entity, true);
		}
	}

	parent->add(*box);
	box->show_all_children();
}

//------------------------------
// Outliner::GetToolbarContent
//
Gtk::Widget* Outliner::GetToolbarContent()
{
	Gtk::Box* const toolbar = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
	toolbar->set_margin_left(20);
	toolbar->set_margin_right(20);

	Gtk::SearchEntry* const searchEntry = Gtk::make_managed<Gtk::SearchEntry>();
	toolbar->pack_start(*searchEntry);
	m_TreeView->set_search_entry(*searchEntry);

	return toolbar;
}

//--------------------------
// Outliner::OnSceneEvent
//
// React to any changes in the scene
//
void Outliner::OnSceneEvent(fw::E_SceneEvent const eventType, fw::SceneEventData const* const eventData)
{
	UNUSED(eventData);

	switch (eventType)
	{
	case fw::E_SceneEvent::Activated:
		RefillTreeView();
		break;
	}
}

//--------------------------
// Outliner::OnSceneEvent
//
// React to any changes in the scene
//
void Outliner::OnEntitySelectionChanged(fw::T_EntityId const entity, bool const selected)
{
	ET_ASSERT(entity != fw::INVALID_ENTITY_ID);

	Gtk::TreeModel::Children children = m_TreeModel->children();

	auto row = RecursiveGetChild(entity, children);

	if (row)
	{
		if (selected)
		{
			m_TreeSelection->select(row);
		}
		else
		{
			m_TreeSelection->unselect(row);
		}
	}
}

//--------------------------------
// Outliner::OnSelectionChanged
//
// Update the selected item list
//
void Outliner::OnEntitySelectionCleared()
{
	m_TreeSelection->unselect_all();
}

//------------------------------------
// Outliner::OnTreeViewRowActivated
//
// When an item in the list gets double clicked / opened
//
void Outliner::OnTreeViewRowActivated(Gtk::TreeModel::Path const& path, Gtk::TreeViewColumn* const column)
{
	UNUSED(column);

	Gtk::TreeModel::iterator const it = m_TreeModel->get_iter(path);
	if (it)
	{
		Gtk::TreeModel::Row row = *it;

		Glib::ustring name = row[m_Columns.m_Name];
		T_Hash id = row[m_Columns.m_Id];

		LOG(FS("Row activated: Name=%s, ID=%u", name.c_str(), id));
	}
}

//--------------------------------
// Outliner::OnSelectionChanged
//
// Update the selected item list
//
void Outliner::OnSelectionChanged()
{
	m_SceneSelection->ClearSelection();

	auto onSelectionIteration = [this](const Gtk::TreeModel::iterator& it)
		{
			fw::T_EntityId const entity = (*it)[m_Columns.m_Id];
			if (entity != fw::INVALID_ENTITY_ID)
			{
				m_SceneSelection->ToggleEntitySelected(entity);
			}
		};

	m_TreeSelection->selected_foreach_iter(onSelectionIteration);
}

//--------------------------
// Outliner::RefillTreeView
//
// Initialize the outliners tree from the Entity hierachy in the current scenes scene graph
//
void Outliner::RefillTreeView()
{
	ET_ASSERT(fw::UnifiedScene::Instance().GetSceneId() != 0u);
	fw::EcsController& ecs = fw::UnifiedScene::Instance().GetEcs();

	Gtk::TreeModel::Row row = *(m_TreeModel->append());
	row[m_Columns.m_Name] = fw::UnifiedScene::Instance().GetSceneName();
	row[m_Columns.m_Id] = fw::INVALID_ENTITY_ID;

	for (fw::T_EntityId const id : ecs.GetEntities())
	{
		// #todo: make this recurse
		ET_ASSERT(ecs.HasComponent<EditorMetaComponent>(id));
		EditorMetaComponent const& meta = ecs.GetComponent<EditorMetaComponent>(id);

		Gtk::TreeModel::iterator iter = m_TreeModel->append(row.children());
		(*iter)[m_Columns.m_Name] = meta.name;
		(*iter)[m_Columns.m_Id] = id;
	}

	m_TreeView->show_all_children();
	m_TreeView->expand_all();
}

//--------------------------
// Outliner::RecursiveGetChild
//
Gtk::TreeModel::Row Outliner::RecursiveGetChild(fw::T_EntityId const entity, Gtk::TreeModel::Children const& children) const
{
	for (Gtk::TreeModel::Children::iterator it = children.begin(); it != children.end(); ++it)
	{
		Gtk::TreeModel::Row row = *it;
		if (row[m_Columns.m_Id] == entity)
		{
			return row;
		}

		row = RecursiveGetChild(entity, it->children());
		if (row)
		{
			return row;
		}
	}

	return *(children.end());
}


} // namespace edit
} // namespace et
