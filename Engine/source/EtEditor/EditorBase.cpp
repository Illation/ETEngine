#include "stdafx.h"
#include "EditorBase.h"

#include <gtkmm/frame.h>
#include <gtkmm/paned.h>

#include <EtEditor/UI/GtkUtil.h>

#include <EtEditor/UI/Outliner.h>
#include <EtEditor/UI/SceneViewport.h>


//===============
// Editor Base
//===============


//---------------------------------
// EditorBase::CreateInnerFrame
//
Gtk::Frame* EditorBase::CreateInnerFrame(Gtk::Paned* const split, bool const isFirst)
{
	Gtk::Frame* childFrame = Gtk::make_managed<Gtk::Frame>();
	childFrame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

	if (isFirst)
	{
		split->add1(*childFrame);
	}
	else
	{
		split->add2(*childFrame);
	}

	return childFrame;
}

//---------------------------------
// EditorBase::OnKeyEvent
//
// by default editors don't handle key events, but this can be overridden by implementations
//
bool EditorBase::OnKeyEvent(bool const pressed, GdkEventKey* const evnt)
{
	UNUSED(pressed);
	UNUSED(evnt);

	return false;
}

//---------------------------------
// EditorBase
//
void EditorBase::CreateTool(E_EditorTool const toolType, Gtk::Frame* const parent)
{
	std::unique_ptr<I_EditorTool> tool;

	switch (toolType)
	{
	case E_EditorTool::SceneViewport:
		tool = std::make_unique<SceneViewport>();
		break;

	case E_EditorTool::Outliner:
		tool = std::make_unique<Outliner>();
		break;

	default:
		ET_ASSERT(true, "unhandled editor tool type");
		break;
	}

	tool->Init(this, parent);

	m_Tools.emplace_back(std::move(tool));
}
