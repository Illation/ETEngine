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
		.property("split ratio", &EditorSplitNode::m_SplitRatio)
		.property("is horizontal", &EditorSplitNode::m_IsHorizontal)
		.property("child 1", &EditorSplitNode::m_Child1)
		.property("child 2", &EditorSplitNode::m_Child2);

	registration::class_<EditorToolNode>("editor tool node")
		.property("type", &EditorToolNode::m_Type);
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

	m_Paned = Gtk::make_managed<Gtk::Paned>(m_IsHorizontal ? Gtk::ORIENTATION_HORIZONTAL : Gtk::ORIENTATION_VERTICAL);
	m_Attachment->add(*m_Paned);

	Gtk::Frame* childFrame1 = Gtk::make_managed<Gtk::Frame>();
	childFrame1->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	m_Paned->add1(*childFrame1);

	Gtk::Frame* childFrame2 = Gtk::make_managed<Gtk::Frame>();
	childFrame2->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	m_Paned->add1(*childFrame2);

	m_Child1->Init(editor, childFrame1);
	m_Child2->Init(editor, childFrame2);
}


//====================
// Editor Tool Node
//====================


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
