#include "stdafx.h"
#include "EditorBase.h"
#include "EditorConfig.h"

#include <gtkmm/frame.h>

#include <EtCore/Reflection/Serialization.h>

#include <EtEditor/UI/GtkUtil.h>
#include <EtEditor/UI/Outliner.h>
#include <EtEditor/UI/SceneViewport.h>


//===============
// Editor Base
//===============


//---------------------------------
// EditorBase::Init
//
// deserialize the default tool layout and construct the tools from it, then init the editor systems
//
void EditorBase::Init(Gtk::Frame* const parent)
{
	std::string const layoutPath = FS("%slayouts/%s.json", EditorConfig::GetInstance()->GetEditorUserDir().c_str(), GetLayoutName().c_str());

	if (!(serialization::DeserializeFromFile(layoutPath, m_NodeHierachy)))
	{
		LOG(FS("Failed to deserialize layout file for '%s' at: %s", GetName().c_str(), layoutPath.c_str()), LogLevel::Warning);
	}

	ET_ASSERT(m_NodeHierachy.root != nullptr);

	m_NodeHierachy.root->Init(this, parent);

	// set ratios upon realize event
	parent->show_all_children();

	InitInternal();
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
