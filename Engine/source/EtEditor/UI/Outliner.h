#pragma once

#include "EditorTool.h"

#include <gtkmm/treestore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/frame.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>

#include <EtEditor/SceneSelection.h>


// forward
namespace Gtk {
	class TreeView;
	class TreeViewColumn;
}


//--------------------
// Outliner
//
// visual representation of the scene hierachy and selection
//
class Outliner final : public I_SceneSelectionListener, public I_EditorTool
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
			add(m_Id);
			add(m_Entity);
		}

		Gtk::TreeModelColumn<Glib::ustring> m_Name;
		Gtk::TreeModelColumn<T_Hash> m_Id;
		Gtk::TreeModelColumn<Entity*> m_Entity;
	};

	// construct destruct
	//--------------------
	Outliner();
	~Outliner();

	// Editor tool interface
	//-----------------------
	void Init(EditorBase* const editor, Gtk::Frame* parent) override;

	// scene selection listener interface
	//--------------------------------------
protected:
	void OnSceneEvent(SceneEventData const* const eventData) override;
	void OnEntitySelectionChanged(Entity* const entity, bool const selected) override;
	void OnEntitySelectionCleared() override;

	// Functionality
	//-------------------
private:
	void OnTreeViewRowActivated(Gtk::TreeModel::Path const& path, Gtk::TreeViewColumn* const column);
	void OnSelectionChanged();

	void RefillTreeView();

	// Utility
	//-------------
	Gtk::TreeModel::Row RecursiveGetChild(Entity* const entity, Gtk::TreeModel::Children const& children) const;

	// Data
	///////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

	Gtk::TreeView* m_TreeView;

	ModelColumns m_Columns;
	Glib::RefPtr<Gtk::TreeStore> m_TreeModel;
	Glib::RefPtr<Gtk::TreeSelection> m_TreeSelection;

	SceneSelection* m_SceneSelection;
};

