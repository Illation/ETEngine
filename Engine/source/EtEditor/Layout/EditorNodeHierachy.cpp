#include "stdafx.h"
#include "EditorNodeHierachy.h"

#include "EditorSplitNode.h"
#include "EditorToolNode.h"
#include "EditorBase.h"

#include <rttr/registration>

#include <glibmm/main.h>
#include <glibmm/object.h>
#include <gtkmm/frame.h>
#include <gtkmm/menu.h>


namespace et {
namespace edit {


//======================
// Editor Node Hierachy
//======================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<EditorNodeHierachy>("editor node hierachy")
		.property("root", &EditorNodeHierachy::m_Root);
}


//---------------------------------
// EditorNodeHierachy::c-tor
//
EditorNodeHierachy::EditorNodeHierachy()
{
	m_RefBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/menu_header.ui");

	Glib::RefPtr<Glib::Object> object = m_RefBuilder->get_object("headermenu");
	m_GMenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
	m_FlipItem = Gio::MenuItem::create("_Flip to top", "header.flip");
	m_GMenu->prepend_item(m_FlipItem);
	m_HeaderMenu = new Gtk::Menu(m_GMenu);
}

//---------------------------------
// EditorNodeHierachy::SplitNode
//
void EditorNodeHierachy::SetHeaderMenuFlipTarget(bool const top)
{
	if (top)
	{
		m_FlipItem->set_label("_Flip to top");
	}
	else
	{
		m_FlipItem->set_label("_Flip to bottom");
	}

	m_GMenu->remove(0);
	m_GMenu->prepend_item(m_FlipItem);
}

//---------------------------------
// EditorNodeHierachy::SplitNode
//
void EditorNodeHierachy::SplitNode(WeakPtr<EditorToolNode> const& node, EditorBase* const editor)
{
	bool const horizontal = node->GetFeedback().GetState() == ToolNodeFeedback::E_State::HSplit;
	float const splitFrac = static_cast<float>(node->GetFeedback().GetSplitPos())
		/ static_cast<float>(horizontal ? node->GetAttachment()->get_allocated_width() : node->GetAttachment()->get_allocated_height());

	node->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);

	WeakPtr<EditorSplitNode> const parent = node->GetParent();
	RefPtr<EditorSplitNode> const split = Create<EditorSplitNode>();

	// find the pointer in the tree referencing our node
	if (parent == nullptr)
	{
		ET_ASSERT(node == m_Root);
		m_Root = split;
		split->m_ThisWeak = m_Root;
	}
	else
	{
		if (node == parent->GetChild1())
		{
			parent->m_Child1 = split;
			split->m_ThisWeak = parent->m_Child1;
		}
		else
		{
			ET_ASSERT(node == parent->GetChild2());
			parent->m_Child2 = split;
			split->m_ThisWeak = parent->m_Child2;
		}
	}

	// assign the basic information that would otherwise be provided by reflection
	RefPtr<EditorToolNode> const splitTool = Create<EditorToolNode>();
	splitTool->m_Type = node->GetType();

	split->m_IsHorizontal = horizontal;
	split->m_SplitRatio = splitFrac;
	split->m_Child1 = node;
	split->m_Child2 = std::move(splitTool);

	// emulate node intialization
	split->m_Attachment = node->UnlinkAttachment();
	split->m_Parent = parent;
	split->InitInternal(editor);

	// mark UI as dirty
	split->GetAttachment()->show_all_children();
	split->AdjustLayout();

	// enusre neighbouring nodes don't allow collapsing the previous neighbour which is now split
	if (split->GetParent() != nullptr)
	{
		split->GetParent()->ReinitHierachyHandles();
	}

	split->GetAttachment()->show_all_children();
}

//----------------------------------
// EditorNodeHierachy::CollapseNode
//
void EditorNodeHierachy::CollapseNode(WeakPtr<EditorToolNode> const& node, EditorBase* const editor)
{
	RefPtr<EditorSplitNode> splitToDelete = node->GetParent(); // ensure we hold the reference, for now
	ET_ASSERT(splitToDelete != nullptr);

	// destroy the tool first in case its destruction relies on being attached to the window (such as open GL contexts etc)
	node->DestroyTool();

	// find the node that doesn't collapse
	RefPtr<EditorNode> survivingNode;
	if (node == splitToDelete->GetChild1())
	{
		survivingNode = splitToDelete->GetChild2();
		splitToDelete->m_Child2 = nullptr;
	}
	else
	{
		ET_ASSERT(node == splitToDelete->GetChild2());
		survivingNode = splitToDelete->GetChild1();
		splitToDelete->m_Child1 = nullptr;
	}

	survivingNode->m_ThisWeak = survivingNode;

	// attatch the surviving node to the parent of the collapsing split node
	RefPtr<EditorSplitNode> survivingParent = splitToDelete->GetParent();
	if (survivingParent == nullptr)
	{
		ET_ASSERT(splitToDelete == m_Root);
		m_Root = survivingNode;
	}
	else
	{
		if (splitToDelete == splitToDelete->GetParent()->GetChild1())
		{
			survivingParent->m_Child1 = survivingNode;
		}
		else
		{
			ET_ASSERT(splitToDelete == splitToDelete->GetParent()->GetChild2());
			survivingParent->m_Child2 = survivingNode;
		}
	}

	// provide the newly attached node with the splits base ui elements
	survivingNode->UnlinkAttachment();
	survivingNode->m_Attachment = splitToDelete->UnlinkAttachment();

	splitToDelete = nullptr; // should be the last reference and memory would be cleared

	survivingNode->m_Parent = survivingParent; // for some reason this needs to happen after invalidating splitToDelete

	// reattach the tool nodes UI
	survivingNode->InitInternal(editor);

	if (survivingNode->GetParent() != nullptr)
	{
		survivingNode->GetParent()->ReinitHierachyHandles();
	}

	survivingNode->GetAttachment()->show_all_children();
}


} // namespace edit
} // namespace et
