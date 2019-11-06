#pragma once

#include "EditorNode.h"
#include "EditorTool.h"
#include "ToolNodeFeedback.h"
#include "ToolHierachyHandle.h"

#include <rttr/registration_friend.h>

#include <glibmm/refptr.h>
#include <giomm/simpleactiongroup.h>


// forward
namespace Gtk {
	class Box;
	class ComboBoxText;
	class Overlay;
	class EventBox;
	class Image;
}
namespace Gdk {
	class Cursor;
}


//---------------------------------
// EditorToolNode
//
// Node that contains a tool
//
class EditorToolNode final : public EditorNode 
{
	// definitions
	//-------------
	RTTR_ENABLE(EditorNode)
	RTTR_REGISTRATION_FRIEND
	friend class EditorNodeHierachy;

	// construct destruct
	//--------------------
public:
	EditorToolNode() : EditorNode() {}
	EditorToolNode(EditorToolNode const& other);
	~EditorToolNode();

	// editor node interface
	//-----------------------
	bool IsLeaf() const override { return true; }
private:
	void InitInternal(EditorBase* const editor) override;

	// functionality
	//---------------
	void CreateTool();
public:
	void DestroyTool();
private:
	void CreateToolbar();
	void OnToolComboChanged();
	void AddToolbarContent();
	void ReorderToolbar();
public:
	void InitHierachyUI();

	// accessors
	//-----------
public:
	E_EditorTool GetType() const { return m_Type; }
	Gtk::Overlay* GetOverlay() const { return m_Overlay; }
	void SetFeedbackState(ToolNodeFeedback::E_State const state);
	ToolNodeFeedback& GetFeedback() { return m_Feedback; }
	EditorBase* GetEditor() const { return m_Editor; }

	// Data
	///////

private:
	// reflected
	E_EditorTool m_Type = E_EditorTool::Invalid;
	bool m_IsToolbarVisible = true;
	bool m_IsToolbarTop = false;

	// model
	std::unique_ptr<I_EditorTool> m_Tool;
	EditorBase* m_Editor = nullptr;

	Glib::RefPtr<Gio::SimpleActionGroup> m_ToolbarActionGroup;

	// ui
	Gtk::Overlay* m_Overlay = nullptr;

	Gtk::Box* m_Container = nullptr;

	Gtk::EventBox* m_ToolbarEventBox = nullptr;
	Gtk::Box* m_Toolbar = nullptr;
	Gtk::ComboBoxText* m_ToolSelector = nullptr;
	Gtk::Widget* m_ToolbarContent = nullptr;

	Gtk::Frame* m_InnerFrame = nullptr;

	Gtk::EventBox* m_ToolbarUnhide = nullptr;
	Gtk::Image* m_ToolbarUnhideImage = nullptr;
	bool m_MouseOverUnhide = false;
	Glib::RefPtr<Gdk::Cursor> m_CursorUnhide;

	ToolHierachyHandle m_Handle1;
	ToolHierachyHandle m_Handle2;

	ToolNodeFeedback m_Feedback;
};

