#pragma once
#include <rttr/type>
#include <rttr/registration_friend.h>

#include <EtEditor/UI/EditorTool.h>


// forward
namespace Gtk {
	class Frame;
	class Paned;
	class Box;
	class ComboBoxText;
}

class EditorNode;


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
	void Init(EditorBase* const editor, Gtk::Frame* const attachment);

	// interface
	//----------------
	virtual bool IsLeaf() const = 0;
protected:
	virtual void InitInternal(EditorBase* const editor) = 0;

	// Data
	///////

	Gtk::Frame* m_Attachment = nullptr;
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

	// Data
	///////

	Gtk::Paned* m_Paned = nullptr;

	float m_SplitRatio = 0.5f;
	bool m_IsHorizontal = true;

	EditorNode* m_Child1 = nullptr;
	EditorNode* m_Child2 = nullptr;

	bool m_LayoutAdjusted = false;
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

	// accessors
	//-----------
public:
	E_EditorTool GetType() const { return m_Type; }

	// Data
	///////

private:
	E_EditorTool m_Type = E_EditorTool::Invalid;
	std::unique_ptr<I_EditorTool> m_Tool;

	Gtk::Box* m_Container = nullptr;
	Gtk::Box* m_Toolbar = nullptr;
	Gtk::ComboBoxText* m_ToolSelector = nullptr;
	Gtk::Frame* m_InnerFrame = nullptr;
};


