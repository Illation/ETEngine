#pragma once
#include <rttr/type>
#include <rttr/registration_friend.h>

#include <EtEditor/UI/EditorTool.h>


// forward
namespace Gtk {
	class Frame;
	class Paned;
}

class EditorNode;


//---------------------------------
// EditorNodeHierachy
//
struct EditorNodeHierachy
{
	EditorNode* root = nullptr;

	RTTR_ENABLE()
};


//---------------------------------
// EditorNode
//
// Abstract node that can contain a hierachy of subdivided tool views
//
class EditorNode
{
public:
	virtual ~EditorNode() = default;

	void Init(EditorBase* const editor, Gtk::Frame* const attachment);

	virtual bool IsLeaf() const = 0;
protected:
	virtual void InitInternal(EditorBase* const editor) = 0;

	Gtk::Frame* m_Attachment = nullptr;

	RTTR_ENABLE()
};


//---------------------------------
// EditorSplitNode
//
// Node that is subdivided into two child nodes
//
class EditorSplitNode final : public EditorNode
{
public:
	EditorSplitNode() : EditorNode() {}
	~EditorSplitNode() = default;

	bool IsLeaf() const override { return false; }
private:
	void InitInternal(EditorBase* const editor) override;

	Gtk::Paned* m_Paned = nullptr;

	float m_SplitRatio = 0.5f;
	bool m_IsHorizontal = true;

	EditorNode* m_Child1 = nullptr;
	EditorNode* m_Child2 = nullptr;

	RTTR_ENABLE(EditorNode)
	RTTR_REGISTRATION_FRIEND
};


//---------------------------------
// EditorToolNode
//
// Node that contains a tool
//
class EditorToolNode final : public EditorNode 
{
public:
	EditorToolNode() : EditorNode() {}
	EditorToolNode(EditorToolNode const& other);
	~EditorToolNode() = default;

	bool IsLeaf() const override { return true; }
private:
	void InitInternal(EditorBase* const editor) override;

public:
	E_EditorTool GetType() const { return m_Type; }

private:

	E_EditorTool m_Type = E_EditorTool::Invalid;
	std::unique_ptr<I_EditorTool> m_Tool;

	RTTR_ENABLE(EditorNode)
	RTTR_REGISTRATION_FRIEND
};


