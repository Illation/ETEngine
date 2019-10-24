#include "stdafx.h"
#include "EditorNode.h"

#include "EditorBase.h"

#include <rttr/registration>

#include <gtkmm/paned.h>
#include <gtkmm/frame.h>

#include <EtEditor/UI/GtkUtil.h>
#include <EtEditor/UI/Outliner.h>
#include <EtEditor/UI/SceneViewport.h>


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

    registration::enumeration<E_EditorTool>("E_EditorTool") (
		value("SceneViewport", E_EditorTool::SceneViewport),
		value("Outliner", E_EditorTool::Outliner),
		value("Invalid", E_EditorTool::Invalid));

	registration::class_<EditorNodeHierachy>("editor node hierachy")
		.property("root", &EditorNodeHierachy::root);

	registration::class_<EditorNode>("editor node");

	registration::class_<EditorSplitNode>("editor split node")
		.constructor<>()(rttr::detail::as_object())
		.property("split ratio", &EditorSplitNode::m_SplitRatio)
		.property("is horizontal", &EditorSplitNode::m_IsHorizontal)
		.property("child 1", &EditorSplitNode::m_Child1)
		.property("child 2", &EditorSplitNode::m_Child2);

	rttr::type::register_converter_func([](EditorSplitNode& node, bool& ok) -> EditorNode*
	{
		ok = true;
		return new EditorSplitNode(node);
	});

	registration::class_<EditorToolNode>("editor tool node")
		.constructor<>()(rttr::detail::as_object())
		.property("type", &EditorToolNode::m_Type);

	rttr::type::register_converter_func([](EditorToolNode& node, bool& ok) -> EditorNode*
	{
		ok = true;
		return new EditorToolNode(node);
	});
}


//==============
// Editor Node
//==============


//---------------------------------
// EditorNode::Init
//
void EditorNode::Init(EditorBase* const editor, Gtk::Frame* const attachment)
{ 
	m_Attachment = attachment;

	InitInternal(editor);
}


//====================
// Editor Split Node
//====================


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
	m_Paned->add1(*childFrame1);

	Gtk::Frame* childFrame2 = Gtk::make_managed<Gtk::Frame>();
	childFrame2->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	m_Paned->add2(*childFrame2);

	// init the child widgets
	m_Child1->Init(editor, childFrame1);
	m_Child2->Init(editor, childFrame2);
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


//====================
// Editor Tool Node
//====================


//---------------------------------
// EditorToolNode::c-tor
//
// Shallow copy constructor implementation due to unique ptr
//
EditorToolNode::EditorToolNode(EditorToolNode const& other) : EditorNode()
{
	m_Type = other.GetType();
}

//---------------------------------
// EditorToolNode::InitInternal
//
void EditorToolNode::InitInternal(EditorBase* const editor)
{
	ET_ASSERT(m_Type != E_EditorTool::Invalid);

	switch (m_Type)
	{
	case E_EditorTool::SceneViewport:
		m_Tool = std::make_unique<SceneViewport>();
		break;

	case E_EditorTool::Outliner:
		m_Tool = std::make_unique<Outliner>();
		break;

	default:
		ET_ASSERT(true, "unhandled editor tool type");
		break;
	}

	m_Tool->Init(editor, m_Attachment);
}
