#include "stdafx.h"
#include "Outliner.h"

#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/treeview.h>

#include <Engine/SceneGraph/AbstractScene.h>
#include <Engine/SceneGraph/Entity.h>


//==========================
// Outliner
//==========================


//--------------------
// Outliner::c-tor
//
Outliner::Outliner(SceneSelection* sceneSelection, Gtk::TreeView* treeView)
	: m_SceneSelection(sceneSelection)
	, m_TreeView(treeView)
{
	m_TreeModel = Gtk::TreeStore::create(m_Columns);

	m_TreeView->set_model(m_TreeModel);

	//All the items to be reordered with drag-and-drop:
	m_TreeView->set_reorderable();

	m_TreeView->signal_row_activated().connect(sigc::mem_fun(*this, &Outliner::OnTreeViewRowActivated));

	m_SceneSelection->RegisterListener(this);
}

//--------------------
// Outliner::d-tor
//
Outliner::~Outliner()
{
	m_SceneSelection->UnregisterListener(this);
}

//--------------------
// Outliner::OnTreeViewRowActivated
//
// When an item in the list gets selected
//
void Outliner::OnTreeViewRowActivated(Gtk::TreeModel::Path const& path, Gtk::TreeViewColumn* const column)
{
	UNUSED(column);

	Gtk::TreeModel::iterator const it = m_TreeModel->get_iter(path);
	if (it)
	{
		Gtk::TreeModel::Row row = *it;

		T_Hash id = row[m_Columns.m_Id];
		Glib::ustring name = row[m_Columns.m_Name];

		LOG(FS("Row activated: ID=%u, Name=%s", id, name.c_str()));
	}
}

//--------------------------
// Outliner::OnSceneEvent
//
// React to any changes in the scene
//
void Outliner::OnSceneEvent(SceneEventData const* const eventData)
{
	ET_ASSERT(eventData != nullptr);

	switch (eventData->eventType)
	{
	case E_SceneEvent::Initialized:
		RefillTreeView();
		break;
	}
}

//--------------------------
// Outliner::RefillTreeView
//
// Initialize the outliners tree from the Entity hierachy in the current scenes scene graph
//
void Outliner::RefillTreeView()
{
	AbstractScene const* const scene = m_SceneSelection->GetScene();
	ET_ASSERT(scene != nullptr);

	Gtk::TreeModel::Row row = *(m_TreeModel->append());
	row[m_Columns.m_Id] = 0;
	row[m_Columns.m_Name] = scene->GetName();

	for (Entity const* const entity : scene->GetEntities())
	{
		Gtk::TreeModel::iterator iter = m_TreeModel->append(row.children());
		(*iter)[m_Columns.m_Id] = entity->GetId();
		(*iter)[m_Columns.m_Name] = entity->GetName();
	}

	m_TreeView->append_column("ID", m_Columns.m_Id);
	m_TreeView->append_column("Name", m_Columns.m_Name);

	m_TreeView->show_all_children();
	m_TreeView->expand_all();
}
