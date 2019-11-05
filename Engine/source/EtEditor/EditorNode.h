#pragma once
#include <rttr/type>
#include <rttr/registration_friend.h>

#include <gdk/gdk.h>
#include <gtkmm/drawingarea.h>

#include <EtEditor/UI/EditorTool.h>


// forward
namespace Gtk {
	class Frame;
	class Paned;
	class Box;
	class ComboBoxText;
	class Overlay;
}
namespace Gdk {
	class Cursor;
}

class EditorNode;
class EditorSplitNode;
class EditorToolNode;


//---------------------------------
// EditorNodeHierachy
//
// Contains for a tree structure of dynamically laid out tools
//
class EditorNodeHierachy
{
	// definitions
	//-------------
	RTTR_ENABLE()

	// functionality
	//----------------
public:
	void SplitNode(EditorToolNode* const node, EditorBase* const editor);
	void CollapseNode(EditorToolNode* const node, EditorBase* const editor);

	// Data
	///////

	EditorNode* root = nullptr;
};


//---------------------------------
// EditorNode
//
// Abstract node that can contain a hierachy of subdivided tool views
//
class EditorNode
{
	// definitions
	//-------------
	RTTR_ENABLE()
	friend class EditorNodeHierachy;

	// construct destruct
	//--------------------
public:
	virtual ~EditorNode() = default;

	// functionality
	//----------------
	void Init(EditorBase* const editor, Gtk::Frame* const attachment, EditorSplitNode* const parent);
	Gtk::Frame* UnlinkAttachment();

	Gtk::Frame* GetAttachment() const { return m_Attachment; }
	EditorSplitNode* GetParent() const { return m_Parent; }

	// accessors
	//-----------
	bool ContainsPointer() const;

	// interface
	//----------------
	virtual bool IsLeaf() const = 0;
protected:
	virtual void InitInternal(EditorBase* const editor) = 0;

	// Data
	///////

	Gtk::Frame* m_Attachment = nullptr;
	EditorSplitNode* m_Parent = nullptr;
};


//---------------------------------
// EditorSplitNode
//
// Node that is subdivided into two child nodes
//
class EditorSplitNode final : public EditorNode
{
	// definitions
	//-------------
	RTTR_ENABLE(EditorNode)
	RTTR_REGISTRATION_FRIEND
	friend class EditorNodeHierachy;

	// construct destruct
	//--------------------
public:
	EditorSplitNode() : EditorNode() {}
	~EditorSplitNode();

	// editor node interface
	//-----------------------
	bool IsLeaf() const override { return false; }
private:
	void InitInternal(EditorBase* const editor) override;

	// functionality
	//----------------
public:
	void AdjustLayout();
	void ReinitHierachyHandles();

	// accessors
	//-----------
	bool IsHorizontal() const { return m_IsHorizontal; }
	EditorNode* GetChild1() const { return m_Child1; }
	EditorNode* GetChild2() const { return m_Child2; }

	// Data
	///////

private:
	Gtk::Paned* m_Paned = nullptr;

	float m_SplitRatio = 0.5f;
	bool m_IsHorizontal = true;

	EditorNode* m_Child1 = nullptr;
	EditorNode* m_Child2 = nullptr;

	bool m_LayoutAdjusted = false;
};


//---------------------------------
// ToolHierachyHandle
//
// Handle added to tool leaf nodes to allow splitting and merging of tools
//
class ToolHierachyHandle final
{
	// definitions
	//-------------
	RTTR_REGISTRATION_FRIEND

	static float const s_SplitThreshold;

	enum class E_DragState
	{
		None,
		Start,
		Abort,

		CollapseNeighbour,
		CollapseOwner,
		CollapseAbort,

		VSplit,
		VSplitAbort,

		HSplit,
		HSplitAbort
	};

	// construct destruct
	//--------------------
public:
	~ToolHierachyHandle() = default;

	// functionality
	//---------------
	void Init(EditorToolNode* const owner, bool right, bool top);

private:
	void ProcessDrag(GdkEventMotion* const motion);
	void ActionDragResult();

	// Data
	///////

	EditorToolNode* m_Owner = nullptr;
	EditorToolNode* m_Neighbour = nullptr;

	bool m_IsRightAligned = false;
	bool m_IsTopAligned = false;

	Glib::RefPtr<Gdk::Cursor> m_CursorCross;

	E_DragState m_DragState = E_DragState::None;
	ivec2 m_Position; // in start state represents the initial position, otherwise the position within the owning frame in case of a split
};


//---------------------------------
// ToolNodeFeedback
//
// Surface to draw into a tools overlat when collapsing or splitting the tool
//
class ToolNodeFeedback final : public Gtk::DrawingArea
{
	// definitions
	//-------------

	static double const s_LineWidth;

public:
	enum class E_State : uint8
	{
		Inactive,
		Collapse,
		HSplit,
		VSplit
	};

	enum class E_Border : uint8
	{
		Left,
		Right,
		Top,
		Bottom
	};

	// construct destruct
	//--------------------
	ToolNodeFeedback() = default;
	~ToolNodeFeedback() = default;

	// drawingArea interface
	//-----------------------
protected:
	bool on_draw(Cairo::RefPtr<Cairo::Context> const& cr) override;

	// accessors
	//---------------
public:
	E_State GetState() const { return m_State; }
	int32 GetSplitPos() const { return m_SplitPos; }

	// functionality
	//---------------
	void SetState(E_State const state) { m_State = state; }
	void SetBorder(E_Border const border) { m_Border = border; }
	void SetSplitPosition(int32 const splitPos) { m_SplitPos = splitPos; }
	bool ForceRedraw();

	// Data
	///////

private:
	E_State m_State = E_State::Inactive;
	E_Border m_Border = E_Border::Left;
	int32 m_SplitPos = 0;
};


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

	// model
	std::unique_ptr<I_EditorTool> m_Tool;
	EditorBase* m_Editor = nullptr;

	// ui
	Gtk::Overlay* m_Overlay = nullptr;
	Gtk::Box* m_Container = nullptr;
	Gtk::Box* m_Toolbar = nullptr;
	Gtk::ComboBoxText* m_ToolSelector = nullptr;
	Gtk::Frame* m_InnerFrame = nullptr;

	ToolHierachyHandle m_Handle1;
	ToolHierachyHandle m_Handle2;

	ToolNodeFeedback m_Feedback;
};
