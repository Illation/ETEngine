#include "stdafx.h"
#include "EditorNode.h"

#include "EditorBase.h"

#include <rttr/registration>

#include <gtkmm/paned.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>

#include <EtCore/Reflection/ReflectionUtil.h>

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
	// space for utility bar with tool switcher
	m_Container = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
	m_Attachment->add(*m_Container);

	// frame for the tool area
	m_InnerFrame = Gtk::make_managed<Gtk::Frame>();
	m_InnerFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

	m_Container->pack_start(*m_InnerFrame);

	// create the tool
	ET_ASSERT(editor != nullptr);
	m_Editor = editor;

	CreateTool();

	CreateToolbar();
}

//---------------------------------
// EditorToolNode::CreateTool
//
void EditorToolNode::CreateTool()
{
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

	m_Tool->Init(m_Editor, m_InnerFrame);
}

//---------------------------------
// EditorToolNode::CreateToolbar
//
void EditorToolNode::CreateToolbar()
{
	// Create the toolbar and a combobox within allowing to select the tool
	m_Toolbar = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
	m_ToolSelector = Gtk::make_managed<Gtk::ComboBoxText>(false);

	m_Toolbar->pack_start(*m_ToolSelector, Gtk::PACK_SHRINK);

	// populate the combobox with all tools that our editor supports
	std::vector<E_EditorTool> const& supportedTools = m_Editor->GetSupportedTools();
	for (E_EditorTool const toolType : supportedTools)
	{
		std::string toolName = reflection::EnumString(toolType);
		m_ToolSelector->append(Glib::ustring(toolName), Glib::ustring(toolName));
	}

	// set our tool current tool as the active element in the combobox
	auto const findResultIt = std::find(supportedTools.cbegin(), supportedTools.cend(), m_Type);
	if (findResultIt != supportedTools.cend())
	{
		m_ToolSelector->set_active(static_cast<int32>(findResultIt - supportedTools.cbegin()));
	}
	else
	{
		ET_ASSERT(false, // warning
			"Current tool type '%s' is not supported by editor '%s'", 
			reflection::EnumString(m_Type).c_str(), 
			m_Editor->GetName().c_str());
	}

	// ensure we get notified when a new tool is selected
	m_ToolSelector->signal_changed().connect(sigc::mem_fun(*this, &EditorToolNode::OnToolComboChanged));

	// add our toolbar to the tool container either in the beginning or the end
	if (m_Tool->IsToolbarTopPref())
	{
		m_Container->pack_start(*m_Toolbar, Gtk::PACK_SHRINK);
	}
	else
	{
		m_Container->pack_end(*m_Toolbar, Gtk::PACK_SHRINK);
	}

	// option to add menus for the tool to the toolbar here
}

//-------------------------------------
// EditorToolNode::OnToolComboChanged
//
// Switch the displayed tool if the user selects a different one
//
void EditorToolNode::OnToolComboChanged()
{
	Glib::ustring toolId = m_ToolSelector->get_active_id();
	if (!(toolId.empty()))
	{
		E_EditorTool const newType = reflection::EnumFromString<E_EditorTool>(toolId.raw());

		if (newType != m_Type)
		{
			m_Type = newType;

			// allow the tool to destroy resources depending on the frame being attached
			m_Tool->OnDeinit();

			// remove the child widget
			Gtk::Widget* child = m_InnerFrame->get_child();
			m_InnerFrame->remove();
			SafeDelete(child);

			// delete the tool
			m_Tool.reset(nullptr);

			// create the new tool
			CreateTool();
		}
	}
}
