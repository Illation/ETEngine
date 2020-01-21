#include "stdafx.h"
#include "EditorToolNode.h"

#include "EditorBase.h"
#include "EditorSplitNode.h"
#include "EditorNodeHierachy.h"

#include <gtkmm/comboboxtext.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/overlay.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gdkmm/cursor.h>

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Reflection/ReflectionUtil.h>

#include <EtEditor/Util/GtkUtil.h>
#include <EtEditor/Tools/Outliner.h>
#include <EtEditor/Tools/SceneViewport.h>


//====================
// Editor Tool Node
//====================


// reflection
RTTR_REGISTRATION
{
	rttr::registration::enumeration<et::edit::E_EditorTool>("E_EditorTool") (
		rttr::value("SceneViewport", et::edit::E_EditorTool::SceneViewport),
		rttr::value("Outliner", et::edit::E_EditorTool::Outliner),
		rttr::value("Invalid", et::edit::E_EditorTool::Invalid));

	BEGIN_REGISTER_POLYMORPHIC_CLASS(et::edit::EditorToolNode, "editor tool node")
		.property("type", &et::edit::EditorToolNode::m_Type)
	END_REGISTER_POLYMORPHIC_CLASS(et::edit::EditorToolNode, et::edit::EditorNode);
}


namespace et {
namespace edit {


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
// EditorToolNode::d-tor
//
EditorToolNode::~EditorToolNode()
{
	if (m_Tool != nullptr)
	{
		DestroyTool();
	}

	if (m_Overlay != nullptr && m_Overlay->get_parent() != nullptr)
	{
		delete m_Overlay; // should also delete other managed wdigets
	}

	if (m_ToolbarEventBox != nullptr && !m_IsToolbarVisible)
	{
		delete m_ToolbarEventBox;
	}

	if (m_ToolbarUnhide != nullptr && m_IsToolbarVisible)
	{
		delete m_ToolbarUnhide;
	}
}

//---------------------------------
// EditorToolNode::InitInternal
//
void EditorToolNode::InitInternal(EditorBase* const editor)
{
	// if the node is being reinitialized to to restructuring of the hierachy, all we do is relink our UI to the attachment
	bool const reinit = m_Overlay != nullptr;

	if (!reinit)
	{
		m_Overlay = Gtk::make_managed<Gtk::Overlay>();
	}
	m_Attachment->add(*m_Overlay);

	if (reinit)
	{
		InitHierachyUI();
		return;
	}

	// space for utility bar with tool switcher
	m_Container = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
	m_Overlay->add(*m_Container);

	// frame for the tool area
	m_InnerFrame = Gtk::make_managed<Gtk::Frame>();
	m_InnerFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

	m_Container->pack_start(*m_InnerFrame);

	// create the tool
	ET_ASSERT(editor != nullptr);
	m_Editor = editor;

	CreateTool();

	CreateToolbar();

	InitHierachyUI();
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

//-------------------------------------
// EditorToolNode::DestroyTool
//
void EditorToolNode::DestroyTool()
{
	// allow the tool to destroy resources depending on the frame being attached
	m_Tool->OnDeinit();

	// remove the child widget
	Gtk::Widget* child = m_InnerFrame->get_child();
	m_InnerFrame->remove();
	SafeDelete(child);

	// delete the tool
	m_Tool.reset(nullptr);

	if (m_ToolbarContent != nullptr)
	{
		m_Toolbar->remove(*m_ToolbarContent);
		SafeDelete(m_ToolbarContent);
	}
}

//---------------------------------
// EditorToolNode::CreateToolbar
//
void EditorToolNode::CreateToolbar()
{
	// Create the toolbar and a combobox within allowing to select the tool
	m_ToolbarEventBox = Gtk::make_managed<Gtk::EventBox>();
	m_Toolbar = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
	m_ToolbarEventBox->add(*m_Toolbar);
	m_Toolbar->set_margin_left(20);
	m_ToolSelector = Gtk::make_managed<Gtk::ComboBoxText>(false);

	m_Toolbar->pack_start(*m_ToolSelector, Gtk::PACK_SHRINK);

	// open a popup menu upon rightclick on the toolbar
	m_ToolbarEventBox->add_events(Gdk::BUTTON_PRESS_MASK);
	auto onButtonPress = [this](GdkEventButton* evnt) -> bool
	{
		if ((evnt->type == GDK_BUTTON_PRESS) && (evnt->button == 3))
		{
			m_Editor->GetHierachy().SetHeaderMenuFlipTarget(!m_IsToolbarTop);
			Gtk::Menu* const menu = m_Editor->GetHierachy().GetHeaderMenu();
			ET_ASSERT(menu != nullptr);

			if (menu->get_attach_widget() == nullptr)
			{
				menu->attach_to_widget(*m_ToolbarEventBox);
			}

			menu->insert_action_group("header", m_ToolbarActionGroup);
			menu->popup(evnt->button, evnt->time);
			return true; 
		}

		return false;
	};
	m_ToolbarEventBox->signal_button_press_event().connect(onButtonPress, false);

	m_ToolbarActionGroup = Gio::SimpleActionGroup::create();

	auto onFlip = [this]()
	{
		m_IsToolbarTop = !m_IsToolbarTop;
		ReorderToolbar();
		InitHierachyUI(); // depends on toolbar position
	};
	m_ToolbarActionGroup->add_action("flip", onFlip);

	auto onShow = [this]()
	{
		m_IsToolbarVisible = !m_IsToolbarVisible;
		ReorderToolbar();
		m_ToolbarActionGroup->lookup("show")->change_state(m_IsToolbarVisible);
	};
	m_ToolbarActionGroup->add_action_bool("show", onShow, m_IsToolbarVisible);

	auto onCollapse = [this]()
	{
		m_Editor->QueueNodeForCollapse(this);
	};
	m_ToolbarActionGroup->add_action("collapse", onCollapse);

	auto onHSplit = [this]()
	{
		m_Feedback.SetState(ToolNodeFeedback::E_State::HSplit);
		m_Feedback.SetSplitPosition(m_Overlay->get_allocated_width() / 2);
		m_Editor->QueueNodeForSplit(this);
	};
	m_ToolbarActionGroup->add_action("hsplit", onHSplit);

	auto onVSplit = [this]()
	{
		m_Feedback.SetState(ToolNodeFeedback::E_State::VSplit);
		m_Feedback.SetSplitPosition(m_Overlay->get_allocated_height() / 2);
		m_Editor->QueueNodeForSplit(this);
	};
	m_ToolbarActionGroup->add_action("vsplit", onVSplit);

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

	// button to unhide the toolbar
	m_ToolbarUnhide = Gtk::make_managed<Gtk::EventBox>();
	m_ToolbarUnhideImage = Gtk::make_managed<Gtk::Image>();
	m_ToolbarUnhide->add(*m_ToolbarUnhideImage);
	m_ToolbarUnhide->set_halign(Gtk::ALIGN_CENTER);
	m_CursorUnhide = Gdk::Cursor::create(m_ToolbarUnhide->get_display(), Gdk::CursorType::DOUBLE_ARROW);

	m_ToolbarUnhide->add_events(Gdk::BUTTON_RELEASE_MASK);
	auto mouseReleasedCallback = [this](GdkEventButton* evnt) -> bool
	{
		if (m_MouseOverUnhide)
		{
			m_IsToolbarVisible = true;
			ReorderToolbar();
			m_ToolbarActionGroup->lookup("show")->change_state(m_IsToolbarVisible);

			return true;
		}
		return true;
	};
	m_ToolbarUnhide->signal_button_release_event().connect(mouseReleasedCallback, false);

	auto mouseEnterCallback = [this](GdkEventCrossing* evnt) -> bool
	{
		m_MouseOverUnhide = true;
		m_ToolbarUnhide->get_window()->set_cursor(m_CursorUnhide);
		return true;
	};
	m_ToolbarUnhide->signal_enter_notify_event().connect(mouseEnterCallback, true);

	auto mouseLeaveCallback = [this](GdkEventCrossing* evnt) -> bool
	{			
		m_ToolbarUnhide->get_window()->set_cursor();
		m_MouseOverUnhide = false;
		return true;
	};
	m_ToolbarUnhide->signal_leave_notify_event().connect(mouseEnterCallback, true);

	// ensure we get notified when a new tool is selected
	m_ToolSelector->signal_changed().connect(sigc::mem_fun(*this, &EditorToolNode::OnToolComboChanged));

	// add our toolbar to the tool container either in the beginning or the end
	m_IsToolbarTop = m_Tool->IsToolbarTopPref();
	ReorderToolbar();

	AddToolbarContent();
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

			DestroyTool();
			CreateTool();
			AddToolbarContent();
		}
	}
}

//-------------------------------------
// EditorToolNode::OnToolComboChanged
//
void EditorToolNode::SetFeedbackState(ToolNodeFeedback::E_State const state)
{
	if ((m_Feedback.GetState() == ToolNodeFeedback::E_State::Inactive) && (state != ToolNodeFeedback::E_State::Inactive))
	{
		m_Overlay->add_overlay(m_Feedback);
		m_Overlay->set_overlay_pass_through(m_Feedback, true);
		m_Overlay->show_all_children();
	}
	else if ((m_Feedback.GetState() != ToolNodeFeedback::E_State::Inactive) && (state == ToolNodeFeedback::E_State::Inactive))
	{
		static_cast<Gtk::Container*>(m_Overlay)->remove(m_Feedback);
	}

	m_Feedback.SetState(state);
}

//-------------------------------------
// EditorToolNode::AddToolbarContent
//
// option to add menus for the tool to the toolbar here
//
void EditorToolNode::AddToolbarContent()
{
	m_ToolbarContent = m_Tool->GetToolbarContent();
	if (m_ToolbarContent != nullptr)
	{
		m_Toolbar->pack_end(*m_ToolbarContent, Gtk::PACK_EXPAND_WIDGET);
		m_Toolbar->show_all_children();
	}
}

//-------------------------------------
// EditorToolNode::ReorderToolbar
//
void EditorToolNode::ReorderToolbar()
{
	// check if we need to add or remove the toolbar
	if (m_IsToolbarVisible)
	{
		if (!(m_ToolbarEventBox->is_ancestor(*m_Container)))
		{
			m_Container->pack_start(*m_ToolbarEventBox, Gtk::PACK_SHRINK);
		}

		if (m_ToolbarUnhide->is_ancestor(*m_Overlay))
		{
			static_cast<Gtk::Container*>(m_Overlay)->remove(*m_ToolbarUnhide);
		}
	}
	else
	{
		if (m_ToolbarEventBox->is_ancestor(*m_Container))
		{
			m_Container->remove(*m_ToolbarEventBox);
		}

		if (!(m_ToolbarUnhide->is_ancestor(*m_Overlay)))
		{
			m_Overlay->add_overlay(*m_ToolbarUnhide);
		}
	}
		
	// change the toolbar position
	if (m_IsToolbarTop)
	{
		m_Container->reorder_child(*m_InnerFrame, 1);
		m_Container->reorder_child(*m_ToolbarEventBox, 0);

		m_ToolbarUnhide->set_valign(Gtk::ALIGN_START);
		m_ToolbarUnhideImage->set_from_resource("/com/leah-lindner/editor/ui/icons/show_header_top.png");
	}
	else
	{
		m_Container->reorder_child(*m_ToolbarEventBox, 1);
		m_Container->reorder_child(*m_InnerFrame, 0); 

		m_ToolbarUnhide->set_valign(Gtk::ALIGN_END);
		m_ToolbarUnhideImage->set_from_resource("/com/leah-lindner/editor/ui/icons/show_header_bot.png");
	}

	m_Overlay->show_all_children();
}

//-------------------------------------
// EditorToolNode::InitHierachyUI
//
void EditorToolNode::InitHierachyUI()
{
	// overlays for splitting and colapsing tools
	m_Handle1.Init(this, false, m_IsToolbarTop);
	m_Handle2.Init(this, true, !m_IsToolbarTop);

	// set the feedback border to determine arrow directions
	if (m_Parent != nullptr)
	{
		if (m_Parent->IsHorizontal())
		{
			m_Feedback.SetBorder((m_Parent->GetChild1() == this) ? ToolNodeFeedback::E_Border::Right : ToolNodeFeedback::E_Border::Left);
		}
		else
		{
			m_Feedback.SetBorder((m_Parent->GetChild1() == this) ? ToolNodeFeedback::E_Border::Bottom : ToolNodeFeedback::E_Border::Top);
		}
	}
}


} // namespace edit
} // namespace et
