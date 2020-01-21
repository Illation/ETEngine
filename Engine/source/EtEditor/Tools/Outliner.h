#pragma once
#include <gtkmm/treestore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/frame.h>
#include <gtkmm/builder.h>
#include <glibmm/refptr.h>

#include <EtEditor/SceneEditor/SceneSelection.h>
#include <EtEditor/Layout/EditorTool.h>


// forward
namespace Gtk {
	class TreeView;
	class TreeViewColumn;
}


namespace et {
namespace edit {


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
		}

		Gtk::TreeModelColumn<Glib::ustring> m_Name;
		Gtk::TreeModelColumn<fw::T_EntityId> m_Id;
	};

	// construct destruct
	//--------------------
	Outliner();
	~Outliner();

	// Editor tool interface
	//-----------------------
	void Init(EditorBase* const editor, Gtk::Frame* parent) override;
	Gtk::Widget* GetToolbarContent() override;

	// scene selection listener interface
	//--------------------------------------
protected:
	void OnSceneEvent(fw::E_SceneEvent const eventType, fw::SceneEventData const* const eventData) override;
	void OnEntitySelectionChanged(fw::T_EntityId const entity, bool const selected) override;
	void OnEntitySelectionCleared() override;

	// Functionality
	//-------------------
private:
	void OnTreeViewRowActivated(Gtk::TreeModel::Path const& path, Gtk::TreeViewColumn* const column);
	void OnSelectionChanged();

	void RefillTreeView();

	// Utility
	//-------------
	Gtk::TreeModel::Row RecursiveGetChild(fw::T_EntityId const entity, Gtk::TreeModel::Children const& children) const;

	// Data
	///////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

	Gtk::TreeView* m_TreeView;

	ModelColumns m_Columns;
	Glib::RefPtr<Gtk::TreeStore> m_TreeModel;
	Glib::RefPtr<Gtk::TreeSelection> m_TreeSelection;

	SceneSelection* m_SceneSelection;
};


} // namespace edit
} // namespace et
