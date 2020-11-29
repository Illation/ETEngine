#include "stdafx.h"
#include "EditorSplitNode.h"

#include "EditorToolNode.h"

#include <gtkmm/frame.h>
#include <gtkmm/paned.h>

#include <EtCore/Reflection/Registration.h>

#include <EtEditor/Util/GtkUtil.h>


//====================
// Editor Split Node
//====================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(et::edit::EditorSplitNode, "editor split node")
		.property("split ratio", &et::edit::EditorSplitNode::m_SplitRatio)
		.property("is horizontal", &et::edit::EditorSplitNode::m_IsHorizontal)
		.property("child 1", &et::edit::EditorSplitNode::m_Child1)
		.property("child 2", &et::edit::EditorSplitNode::m_Child2)
	END_REGISTER_CLASS_POLYMORPHIC(et::edit::EditorSplitNode, et::edit::EditorNode);
}


namespace et {
namespace edit {


//---------------------------------
// EditorSplitNode::d-tor
//
EditorSplitNode::~EditorSplitNode()
{
	if (m_Paned == nullptr)
	{
		return;
	}

	// in case of collapsing, the surviving child should be set to nullptr in the split before calling the destructor
	SafeDelete(m_Child1);
	SafeDelete(m_Child2);

	// if this nodes UI is unlinked from its attachment, we need to manually delete the UI
	if (m_Paned->get_parent() == nullptr)
	{
		delete m_Paned;
	}
}

//---------------------------------
// EditorSplitNode::InitInternal
//
void EditorSplitNode::InitInternal(EditorBase* const editor)
{
	ET_ASSERT(m_Child1 != nullptr);
	ET_ASSERT(m_Child2 != nullptr);

	// create a paned widget which handles splitting the frame
	m_Paned = Gtk::make_managed<Gtk::Paned>(m_IsHorizontal ? Gtk::ORIENTATION_HORIZONTAL : Gtk::ORIENTATION_VERTICAL);
	m_Attachment->add(*m_Paned);

	// attatch inner frames to the paned widget in order to create shadowing
	Gtk::Frame* childFrame1 = Gtk::make_managed<Gtk::Frame>();
	childFrame1->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	m_Paned->pack1(*childFrame1, true, true);

	Gtk::Frame* childFrame2 = Gtk::make_managed<Gtk::Frame>();
	childFrame2->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	m_Paned->pack2(*childFrame2, true, true);

	// init the child widgets
	m_Child1->Init(editor, childFrame1, this);
	m_Child2->Init(editor, childFrame2, this);
}

//---------------------------------
// EditorSplitNode::AdjustLayout
//
// Set the pane position once the widget is realized
//
void EditorSplitNode::AdjustLayout()
{
	if (m_LayoutAdjusted)
	{
		return;
	}

	// set the position of the handle
	int32 size = 0;

	if (m_IsHorizontal)
	{
		size = m_Attachment->get_allocated_width();
	}
	else
	{
		size = m_Attachment->get_allocated_height();
	}

	m_Paned->set_position(static_cast<int32>(m_SplitRatio * static_cast<float>(size)));

	// propagate this to children
	if (!(m_Child1->IsLeaf()))
	{
		static_cast<EditorSplitNode*>(m_Child1)->AdjustLayout();
	}

	if (!(m_Child2->IsLeaf()))
	{
		static_cast<EditorSplitNode*>(m_Child2)->AdjustLayout();
	}

	m_LayoutAdjusted = true;

	// ensure the split ratio is updated if the user drags the handle
	auto positionChangedCallback = [this]()
	{
		if (!m_LayoutAdjusted)
		{
			return;
		}

		float size = 0.f;
		if (m_IsHorizontal)
		{
			size = static_cast<float>(m_Attachment->get_allocated_width());
		}
		else
		{
			size = static_cast<float>(m_Attachment->get_allocated_height());
		}

		if (size == 0.f)
		{
			return;
		}

		m_SplitRatio = static_cast<float>(m_Paned->get_position()) / size;
	};
	m_Paned->property_position().signal_changed().connect(positionChangedCallback);
}

//----------------------------------------
// EditorSplitNode::ReinitHierachyHandles
//
void EditorSplitNode::ReinitHierachyHandles()
{
	if (m_Child1->IsLeaf())
	{
		static_cast<EditorToolNode*>(m_Child1)->InitHierachyUI();
	}

	if (m_Child2->IsLeaf())
	{
		static_cast<EditorToolNode*>(m_Child2)->InitHierachyUI();
	}
}


} // namespace edit
} // namespace et
