#pragma once
#include <rttr/type>
#include <rttr/registration_friend.h>

#include <gdk/gdk.h>

#include <EtEditor/UI/EditorTool.h>


// forward
namespace Gtk {
	class Frame;
	class Paned;
	class Box;
	class ComboBoxText;
	class Overlay;
}

class EditorNode;
class EditorSplitNode;
class EditorToolNode;


//---------------------------------
// EditorNodeHierachy
//
// Serialization wrapper
//
struct EditorNodeHierachy
{
	RTTR_ENABLE()

public:
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

	// construct destruct
	//--------------------
public:
	virtual ~EditorNode() = default;

	// functionality
	//----------------
	void Init(EditorBase* const editor, Gtk::Frame* const attachment, EditorSplitNode* const parent);

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

	// construct destruct
	//--------------------
public:
	EditorSplitNode() : EditorNode() {}
	~EditorSplitNode() = default;

	// editor node interface
	//-----------------------
	bool IsLeaf() const override { return false; }
private:
	void InitInternal(EditorBase* const editor) override;

	// functionality
	//----------------
public:
	void AdjustLayout();

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
	void Init(Gtk::Overlay* const attachment, EditorToolNode* const owner, bool right, bool top);

private:
	void ProcessDrag(GdkEventMotion* const motion);
	void ActionDragResult();

	// Data
	///////

	EditorToolNode* m_Owner = nullptr;
	EditorToolNode* m_Neighbour = nullptr;

	bool m_IsRightAligned = false;
	bool m_IsTopAligned = false;

	E_DragState m_DragState = E_DragState::None;
	ivec2 m_Position; // in start state represents the initial position, otherwise the position within the owning frame in case of a split
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

	// construct destruct
	//--------------------
public:
	EditorToolNode() : EditorNode() {}
	EditorToolNode(EditorToolNode const& other);
	~EditorToolNode() = default;

	// editor node interface
	//-----------------------
	bool IsLeaf() const override { return true; }
private:
	void InitInternal(EditorBase* const editor) override;

	// functionality
	//---------------
	void CreateTool();
	void CreateToolbar();
	void OnToolComboChanged();

	// accessors
	//-----------
public:
	E_EditorTool GetType() const { return m_Type; }

	// Data
	///////

private:
	// reflected
	E_EditorTool m_Type = E_EditorTool::Invalid;

	// model
	std::unique_ptr<I_EditorTool> m_Tool;
	EditorBase* m_Editor = nullptr;

	// ui
	Gtk::Box* m_Container = nullptr;
	Gtk::Box* m_Toolbar = nullptr;
	Gtk::ComboBoxText* m_ToolSelector = nullptr;
	Gtk::Frame* m_InnerFrame = nullptr;

	ToolHierachyHandle m_Handle1;
	ToolHierachyHandle m_Handle2;
};
