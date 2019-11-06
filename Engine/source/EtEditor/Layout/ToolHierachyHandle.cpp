#include "stdafx.h"
#include "ToolHierachyHandle.h"

#include "EditorBase.h"
#include "EditorSplitNode.h"
#include "EditorToolNode.h"

#include <gtkmm/frame.h>
#include <gtkmm/overlay.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gdkmm/cursor.h>

#include <EtEditor/Util/GtkUtil.h>


//======================
// Tool Hierachy Handle
//======================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;
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
}


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
		SetCornerImage();
		return;
	}

	// create thw events
	m_EventBox = Gtk::make_managed<Gtk::EventBox>();
	m_Owner->GetOverlay()->add_overlay(*m_EventBox);

	m_EventBox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);

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
	m_EventBox->signal_button_press_event().connect(mousePressedCallback, false);

	auto mouseReleasedCallback = [this](GdkEventButton* evnt) -> bool
	{
		ActionDragResult();

		m_DragState = E_DragState::None;
		m_EventBox->get_window()->set_cursor();
		return true;
	};
	m_EventBox->signal_button_release_event().connect(mouseReleasedCallback, false);

	auto mouseMotionCallback = [this](GdkEventMotion* evnt) -> bool
	{
		if (m_DragState != E_DragState::None)
		{
			ProcessDrag(evnt);

			return true;
		}

		return false;
	};
	m_EventBox->signal_motion_notify_event().connect(mouseMotionCallback, false);

	m_CursorCross = Gdk::Cursor::create(m_EventBox->get_display(), Gdk::CursorType::DIAMOND_CROSS);

	auto mouseEnterCallback = [this](GdkEventCrossing* evnt) -> bool
	{
		m_EventBox->get_window()->set_cursor(m_CursorCross);
		return true;
	};
	m_EventBox->signal_enter_notify_event().connect(mouseEnterCallback, true);

	auto mouseLeaveCallback = [this](GdkEventCrossing* evnt) -> bool
	{
		if (m_DragState != E_DragState::None)
		{
			m_EventBox->get_window()->set_cursor();
		}
		return true;
	};
	m_EventBox->signal_leave_notify_event().connect(mouseEnterCallback, true);

	// create the icon
	m_Image = Gtk::make_managed<Gtk::Image>();
	m_EventBox->add(*m_Image);
	SetCornerImage();
}

//------------------------------------
// ToolHierachyHandle::SetCornerImage
//
void ToolHierachyHandle::SetCornerImage()
{
	if (m_IsTopAligned)
	{
		if (m_IsRightAligned)
		{
			m_Image->set_from_resource("/com/leah-lindner/editor/ui/icons/tool_hierachy_control_tr.png");
		}
		else
		{
			m_Image->set_from_resource("/com/leah-lindner/editor/ui/icons/tool_hierachy_control_tl.png");
		}
	}
	else
	{
		if (m_IsRightAligned)
		{
			m_Image->set_from_resource("/com/leah-lindner/editor/ui/icons/tool_hierachy_control_br.png");
		}
		else
		{
			m_Image->set_from_resource("/com/leah-lindner/editor/ui/icons/tool_hierachy_control_bl.png");
		}
	}

	m_EventBox->set_halign(m_IsRightAligned ? Gtk::ALIGN_END : Gtk::ALIGN_START);
	m_EventBox->set_valign(m_IsTopAligned ? Gtk::ALIGN_START : Gtk::ALIGN_END);
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

