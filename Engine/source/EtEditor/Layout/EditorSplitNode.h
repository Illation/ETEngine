#pragma once
#include "EditorNode.h"

#include <rttr/registration_friend.h>


// forward
namespace Gtk {
	class Paned;
}


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

