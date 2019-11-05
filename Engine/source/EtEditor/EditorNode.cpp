#include "stdafx.h"
#include "EditorNode.h"

#include "EditorBase.h"

#include <rttr/registration>

#include <gtkmm/overlay.h>
#include <gtkmm/paned.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gtkmm/window.h>
#include <gtkmm/menu.h>
#include <gdkmm/cursor.h>
#include <giomm/menu.h>
#include <glibmm/main.h>
#include <glibmm/object.h>
#include <cairomm/context.h>

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

	registration::enumeration<ToolHierachyHandle::E_DragState>("E_DragState") (
		value("None", ToolHierachyHandle::E_DragState::None),
		value("Start", ToolHierachyHandle::E_DragState::Start),
		value("Abort", ToolHierachyHandle::E_DragState::Abort),
		value("Collapse Neighbour", ToolHierachyHandle::E_DragState::CollapseNeighbour),
		value("Collapse Owner", ToolHierachyHandle::E_DragState::CollapseOwner),
		value("Collapse Abort", ToolHierachyHandle::E_DragState::CollapseAbort),
		value("Vertical Split", ToolHierachyHandle::E_DragState::VSplit),
		value("Vertical Split Abort", ToolHierachyHandle::E_DragState::VSplitAbort),
		value("Horizontal Split", ToolHierachyHandle::E_DragState::HSplit),
		value("Horizontal Split Abort", ToolHierachyHandle::E_DragState::HSplitAbort));

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


//======================
// Editor Node Hierachy
//======================


//---------------------------------
// EditorNodeHierachy::c-tor
//
EditorNodeHierachy::EditorNodeHierachy()
{
	m_RefBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/menu_header.ui");

	Glib::RefPtr<Glib::Object> object = m_RefBuilder->get_object("headermenu");
	Glib::RefPtr<Gio::Menu> gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
	m_HeaderMenu = new Gtk::Menu(gmenu);
}

//---------------------------------
// EditorNodeHierachy::SplitNode
//
void EditorNodeHierachy::SplitNode(EditorToolNode* const node, EditorBase* const editor)
{
	bool const horizontal = node->GetFeedback().GetState() == ToolNodeFeedback::E_State::HSplit;
	float const splitFrac = static_cast<float>(node->GetFeedback().GetSplitPos())
		/ static_cast<float>(horizontal ? node->GetAttachment()->get_allocated_width() : node->GetAttachment()->get_allocated_height());

	node->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);

	EditorSplitNode* const split = new EditorSplitNode();

	// find the pointer in the tree referencing our node
	if (node->GetParent() == nullptr)
	{
		ET_ASSERT(node == root);
		root = split;
	}
	else
	{
		if (node == node->GetParent()->GetChild1())
		{
			node->GetParent()->m_Child1 = split;
		}
		else
		{
			ET_ASSERT(node == node->GetParent()->GetChild2());
			node->GetParent()->m_Child2 = split;
		}
	}

	// assign the basic information that would otherwise be provided by reflection
	EditorToolNode* const splitTool = new EditorToolNode();
	splitTool->m_Type = node->GetType();

	split->m_IsHorizontal = horizontal;
	split->m_SplitRatio = splitFrac;
	split->m_Child1 = node;
	split->m_Child2 = splitTool;

	// emulate node intialization
	split->m_Attachment = node->UnlinkAttachment();
	split->m_Parent = node->GetParent();
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
void EditorNodeHierachy::CollapseNode(EditorToolNode* const node, EditorBase* const editor)
{
	EditorSplitNode* splitToDelete = node->GetParent();
	ET_ASSERT(splitToDelete != nullptr);

	// destroy the tool first in case its destruction relies on being attached to the window (such as open GL contexts etc)
	node->DestroyTool();

	// find the node that doesn't collapse
	EditorNode* survivingNode = nullptr;
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

	// attatch the surviving node to the parent of the collapsing split node
	if (splitToDelete->GetParent() == nullptr)
	{
		ET_ASSERT(splitToDelete == root);
		root = survivingNode;
	}
	else
	{
		if (splitToDelete == splitToDelete->GetParent()->GetChild1())
		{
			splitToDelete->GetParent()->m_Child1 = survivingNode;
		}
		else
		{
			ET_ASSERT(splitToDelete == splitToDelete->GetParent()->GetChild2());
			splitToDelete->GetParent()->m_Child2 = survivingNode;
		}
	}

	// provide the newly attached node with the splits base ui elements
	survivingNode->UnlinkAttachment();
	survivingNode->m_Attachment = splitToDelete->UnlinkAttachment();
	survivingNode->m_Parent = splitToDelete->GetParent();

	SafeDelete(splitToDelete);

	// reattach the tool nodes UI
	survivingNode->InitInternal(editor);

	if (survivingNode->GetParent() != nullptr)
	{
		survivingNode->GetParent()->ReinitHierachyHandles();
	}

	survivingNode->GetAttachment()->show_all_children();
}


//==============
// Editor Node
//==============


//---------------------------------
// EditorNode::Init
//
void EditorNode::Init(EditorBase* const editor, Gtk::Frame* const attachment, EditorSplitNode* const parent)
{ 
	m_Attachment = attachment;
	m_Parent = parent;

	InitInternal(editor);
}

//---------------------------------
// EditorNode::UnlinkAttachment
//
// In order to reparent a node, detatch a nodes internals from its current attachment
//
Gtk::Frame* EditorNode::UnlinkAttachment()
{
	m_Attachment->remove();

	Gtk::Frame* const attachment = m_Attachment;
	m_Attachment = nullptr;

	return attachment;
}

//---------------------------------
// EditorNode::ContainsPointer
//
bool EditorNode::ContainsPointer() const
{
	ivec2 pos;
	m_Attachment->get_pointer(pos.x, pos.y);
	Gtk::Allocation const alloc = m_Attachment->get_allocation();

	return (pos.x > 0 && pos.y > 0 && pos.x < alloc.get_width() && pos.y < alloc.get_height());
}


//====================
// Editor Split Node
//====================


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


//======================
// Tool Hierachy Handle
//======================


float const ToolHierachyHandle::s_SplitThreshold = 20.f;


//-------------------------------------
// ToolHierachyHandle::Init
//
void ToolHierachyHandle::Init(EditorToolNode* const owner, bool right, bool top)
{
	// set internals
	ET_ASSERT(owner != nullptr);

	bool const reinit = m_Owner != nullptr;

	m_Owner = owner;
	m_IsRightAligned = right;
	m_IsTopAligned = top;

	// find any neighbouring nodes
	m_Neighbour = nullptr;
	EditorSplitNode* const parentSplit = m_Owner->GetParent();
	if (parentSplit != nullptr)
	{
		EditorNode* testNeighbour = nullptr;

		// figure out which node to check based on our alignment and parent split orientation
		if (parentSplit->IsHorizontal())
		{
			if (m_IsRightAligned)
			{
				testNeighbour = parentSplit->GetChild2();
			}
			else
			{
				testNeighbour = parentSplit->GetChild1();
			}
		}
		else
		{
			if (m_IsTopAligned)
			{
				testNeighbour = parentSplit->GetChild1();
			}
			else
			{
				testNeighbour = parentSplit->GetChild2();
			}
		}

		// we can only merge with leaf nodes, and if this is an outer handle the node we are testing will be the owner
		ET_ASSERT(testNeighbour != nullptr);
		if (testNeighbour != m_Owner && testNeighbour->IsLeaf())
		{
			m_Neighbour = static_cast<EditorToolNode*>(testNeighbour);
		}
	}

	// we should have already created our child widgets and hooked up events previously
	if (reinit)
	{
		return;
	}

	// create thw events
	Gtk::EventBox* const eventBox = Gtk::make_managed<Gtk::EventBox>();
	m_Owner->GetOverlay()->add_overlay(*eventBox);
	eventBox->set_halign(m_IsRightAligned ? Gtk::ALIGN_END : Gtk::ALIGN_START);
	eventBox->set_valign(m_IsTopAligned ? Gtk::ALIGN_START : Gtk::ALIGN_END);

	eventBox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);

	auto mousePressedCallback = [this](GdkEventButton* evnt) -> bool
	{
		m_DragState = E_DragState::Start;

		// initial mouse position - coord space doesn't matter as long as threwhold check uses same
		m_Position = etm::vecCast<int32>(dvec2(evnt->x, evnt->y));
		m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
		if (m_Neighbour != nullptr)
		{
			m_Neighbour->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
		}

		return true;
	};
	eventBox->signal_button_press_event().connect(mousePressedCallback, false);

	auto mouseReleasedCallback = [this, eventBox](GdkEventButton* evnt) -> bool
	{
		ActionDragResult();

		m_DragState = E_DragState::None;
		eventBox->get_window()->set_cursor();
		return true;
	};
	eventBox->signal_button_release_event().connect(mouseReleasedCallback, false);

	auto mouseMotionCallback = [this, eventBox](GdkEventMotion* evnt) -> bool
	{
		if (m_DragState != E_DragState::None)
		{
			ProcessDrag(evnt);

			return true;
		}

		return false;
	};
	eventBox->signal_motion_notify_event().connect(mouseMotionCallback, false);

	m_CursorCross = Gdk::Cursor::create(eventBox->get_display(), Gdk::CursorType::DIAMOND_CROSS);

	auto mouseEnterCallback = [this, eventBox](GdkEventCrossing* evnt) -> bool
	{
		eventBox->get_window()->set_cursor(m_CursorCross);
		return true;
	};
	eventBox->signal_enter_notify_event().connect(mouseEnterCallback, true);

	auto mouseLeaveCallback = [this, eventBox](GdkEventCrossing* evnt) -> bool
	{
		if (m_DragState != E_DragState::None)
		{
			eventBox->get_window()->set_cursor();
		}
		return true;
	};
	eventBox->signal_leave_notify_event().connect(mouseEnterCallback, true);

	// create the icon
	Gtk::Image* const image = Gtk::make_managed<Gtk::Image>();
	eventBox->add(*image);
	if (m_IsTopAligned)
	{
		if (m_IsRightAligned)
		{
			image->set_from_resource("/com/leah-lindner/editor/ui/icons/tool_hierachy_control_tr.png");
		}
		else
		{
			image->set_from_resource("/com/leah-lindner/editor/ui/icons/tool_hierachy_control_tl.png");
		}
	}
	else
	{
		if (m_IsRightAligned)
		{
			image->set_from_resource("/com/leah-lindner/editor/ui/icons/tool_hierachy_control_br.png");
		}
		else
		{
			image->set_from_resource("/com/leah-lindner/editor/ui/icons/tool_hierachy_control_bl.png");
		}
	}
}
//---------------------------------
// ToolHierachyHandle::ProcessDrag
//
// figure out drag state and position
//
void ToolHierachyHandle::ProcessDrag(GdkEventMotion* const motion)
{
	switch (m_DragState)
	{
	case E_DragState::Start:
	{
		// Check if we can collapse onto our neighbour
		if (m_Neighbour != nullptr && m_Neighbour->ContainsPointer())
		{
			m_DragState = E_DragState::CollapseNeighbour;
			break;
		}

		// otherwise check if we surpass the threshold to split the node
		vec2 const mouseDelta = etm::vecCast<float>(dvec2(motion->x, motion->y)) - etm::vecCast<float>(m_Position);
		if (etm::length(mouseDelta) > s_SplitThreshold)
		{
			if (m_Owner->ContainsPointer())
			{
				if (std::abs(mouseDelta.x) > std::abs(mouseDelta.y))
				{
					m_DragState = E_DragState::HSplit;
				}
				else
				{
					m_DragState = E_DragState::VSplit;
				}
			}
			else
			{
				m_DragState = E_DragState::Abort;
			}
		}

		break;
	}

	case E_DragState::Abort:
		// Do nothing, just wait for mouse release
		break;

	case E_DragState::CollapseNeighbour:
	case E_DragState::CollapseOwner:
	case E_DragState::CollapseAbort:
		if (m_Neighbour->ContainsPointer())
		{
			m_DragState = E_DragState::CollapseNeighbour;

			m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
			m_Neighbour->SetFeedbackState(ToolNodeFeedback::E_State::Collapse);
		}
		else if (m_Owner->ContainsPointer())
		{
			m_DragState = E_DragState::CollapseOwner;

			m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::Collapse);
			m_Neighbour->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
		}
		else
		{
			m_DragState = E_DragState::CollapseAbort;

			m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
			m_Neighbour->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
		}

		break;

	case E_DragState::VSplit:
	case E_DragState::VSplitAbort:
		if (m_Owner->ContainsPointer())
		{
			m_DragState = E_DragState::VSplit;
			m_Owner->GetAttachment()->get_pointer(m_Position.x, m_Position.y);

			m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::VSplit);
			m_Owner->GetFeedback().SetSplitPosition(m_Position.y);
		}
		else
		{
			m_DragState = E_DragState::VSplitAbort;
			m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
		}

		break;

	case E_DragState::HSplit:
	case E_DragState::HSplitAbort:
		if (m_Owner->ContainsPointer())
		{
			m_DragState = E_DragState::HSplit;
			m_Owner->GetAttachment()->get_pointer(m_Position.x, m_Position.y);

			m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::HSplit);
			m_Owner->GetFeedback().SetSplitPosition(m_Position.x);
		}
		else
		{
			m_DragState = E_DragState::HSplitAbort;
			m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
		}

		break;

	default:
		ET_ASSERT(true, "Unhandled drag state!"); // maybe the state wasn't reset correctly or this function was called illegally
	}

	m_Owner->GetFeedback().ForceRedraw();
	if (m_Neighbour != nullptr)
	{
		m_Neighbour->GetFeedback().ForceRedraw();
	}
}

//---------------------------------
// ToolHierachyHandle::ActionDragResult
//
// Based on the current drag state, split the node, collapse the node or do nothing
//
void ToolHierachyHandle::ActionDragResult()
{
	switch (m_DragState)
	{
	case E_DragState::VSplit:
	case E_DragState::HSplit:
		m_Owner->GetEditor()->QueueNodeForSplit(m_Owner); // the split code will read the feedback and reset it after
		break;

	case E_DragState::CollapseNeighbour:
		ET_ASSERT(m_Neighbour != nullptr);

		m_Neighbour->SetFeedbackState(ToolNodeFeedback::E_State::Inactive); // no feedback state read required for collapse
		m_Neighbour->GetEditor()->QueueNodeForCollapse(m_Neighbour);
		break;

	case E_DragState::CollapseOwner:
		m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::Inactive); // no feedback state read required for collapse
		m_Owner->GetEditor()->QueueNodeForCollapse(m_Owner);
		break;

	default:
		m_Owner->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
		if (m_Neighbour != nullptr)
		{
			m_Neighbour->SetFeedbackState(ToolNodeFeedback::E_State::Inactive);
		}
		break;
	}
}


//====================
// Tool Node Feedback
//====================


double const ToolNodeFeedback::s_LineWidth = 2.0;


//---------------------------------
// ToolNodeFeedback::on_draw
//
bool ToolNodeFeedback::on_draw(Cairo::RefPtr<Cairo::Context> const& cr)
{
	Gtk::Allocation const allocation = get_allocation();
	double const width = static_cast<double>(allocation.get_width());
	double const height = static_cast<double>(allocation.get_height());

	cr->push_group();
	cr->set_source_rgba(0.0, 0.0, 0.0, 0.5);
	cr->rectangle(0.0, 0.0, width, height);
	cr->fill();

	// draw a split line
	cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
	cr->set_line_width(s_LineWidth);
	if (m_State == E_State::HSplit)
	{
		cr->move_to(static_cast<double>(m_SplitPos), 0.0);
		cr->line_to(static_cast<double>(m_SplitPos), height);
		cr->stroke();
	}
	else if (m_State == E_State::VSplit)
	{
		cr->move_to(0.0, static_cast<double>(m_SplitPos));
		cr->line_to(width, static_cast<double>(m_SplitPos));
		cr->stroke();
	}
	else if (m_State == E_State::Collapse)
	{
		// generate the path for a filled arrow
		cr->move_to(width * 0.5, height * 0.5);
		switch (m_Border)
		{
		case E_Border::Left:
			cr->line_to(width * 0.25, height * 0.25);
			cr->line_to(width * 0.25, height * 0.375);
			cr->line_to(width * 0.0, height * 0.375);
			cr->line_to(width * 0.0, height * 0.625);
			cr->line_to(width * 0.25, height * 0.625);
			cr->line_to(width * 0.25, height * 0.75);
			break;

		case E_Border::Right:
			cr->line_to(width * 0.75, height * 0.25);
			cr->line_to(width * 0.75, height * 0.375);
			cr->line_to(width * 1.0, height * 0.375);
			cr->line_to(width * 1.0, height * 0.625);
			cr->line_to(width * 0.75, height * 0.625);
			cr->line_to(width * 0.75, height * 0.75);
			break;

		case E_Border::Top:
			cr->line_to(width * 0.25, height * 0.25);
			cr->line_to(width * 0.375, height * 0.25);
			cr->line_to(width * 0.375, height * 0.0);
			cr->line_to(width * 0.625, height * 0.0);
			cr->line_to(width * 0.625, height * 0.25);
			cr->line_to(width* 0.75, height * 0.25);
			break;

		case E_Border::Bottom:
			cr->line_to(width * 0.25, height * 0.75);
			cr->line_to(width * 0.375, height * 0.75);
			cr->line_to(width * 0.375, height * 1.0);
			cr->line_to(width * 0.625, height * 1.0);
			cr->line_to(width * 0.625, height * 0.75);
			cr->line_to(width * 0.75, height * 0.75);
			break;
		}
		cr->close_path();

		// draw the arrow, subtracting from the surface
		Cairo::Operator const opDefault = cr->get_operator();

		cr->set_operator(Cairo::Operator::OPERATOR_DEST_OUT);
		cr->set_source_rgba(1.0, 1.0, 1.0, 0.5);
		cr->fill();

		cr->set_operator(opDefault);
	}

	cr->pop_group_to_source();
	cr->rectangle(0.0, 0.0, width, height);
	cr->fill();

	return true;
}

//---------------------------------
// ToolNodeFeedback::ForceRedraw
//
bool ToolNodeFeedback::ForceRedraw()
{
	auto win = get_window();
	if (win)
	{
		Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
		win->invalidate_rect(r, false);
	}

	return true;
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
			Gtk::Menu* menu = m_Editor->GetHierachy().GetHeaderMenu();
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
		LOG("Flipped header");
	};
	m_ToolbarActionGroup->add_action("flip", onFlip);

	auto onShow = [this]()
	{
		if (m_IsToolbarVisible)
		{
			LOG("Hiding header");
		}
		else
		{
			LOG("Showing header");
		}
		m_IsToolbarVisible = !m_IsToolbarVisible;
		m_ToolbarActionGroup->lookup("show")->change_state(m_IsToolbarVisible);
	};
	m_ToolbarActionGroup->add_action_bool("show", onShow, m_IsToolbarVisible);

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
		m_Container->pack_start(*m_ToolbarEventBox, Gtk::PACK_SHRINK);
	}
	else
	{
		m_Container->pack_end(*m_ToolbarEventBox, Gtk::PACK_SHRINK);
	}

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
// EditorToolNode::InitHierachyUI
//
void EditorToolNode::InitHierachyUI()
{
	// overlays for splitting and colapsing tools
	m_Handle1.Init(this, false, false);
	m_Handle2.Init(this, true, true);

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
