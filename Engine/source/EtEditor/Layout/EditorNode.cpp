#include "stdafx.h"
#include "EditorNode.h"

#include "EditorBase.h"

#include <rttr/registration>

#include <gtkmm/frame.h>


//==============
// Editor Node
//==============


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<EditorNode>("editor node");
}


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

