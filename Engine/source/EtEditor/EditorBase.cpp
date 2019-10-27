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
	// load the tool hierachy from a layout file
	std::string const layoutPath = FS("%slayouts/%s.json", EditorConfig::GetInstance()->GetEditorUserDir().c_str(), GetLayoutName().c_str());

	if (!(serialization::DeserializeFromFile(layoutPath, m_NodeHierachy)))
	{
		LOG(FS("Failed to deserialize layout file for '%s' at: %s", GetName().c_str(), layoutPath.c_str()), LogLevel::Warning);
	}

	ET_ASSERT(m_NodeHierachy.root != nullptr);

	// initialize the tools and gtk widgets based on the loaded hierachy
	m_NodeHierachy.root->Init(this, parent);

	// once the widget sizes are available, adjust the sizes of widgets based on what the layout loaded
	auto allocateCallback = [this](Gtk::Allocation& allocation)
	{
		UNUSED(allocation);

		m_HasInitialSize = true;
	};
	parent->signal_size_allocate().connect(allocateCallback, true);

	// ..
	parent->show_all_children();

	// let the derived editor do any initialization it needs
	InitInternal();
}

//---------------------------------
// EditorBase::SaveLayout
//
void EditorBase::SaveLayout()
{
	std::string const layoutPath = FS("%slayouts/%s.json", EditorConfig::GetInstance()->GetEditorUserDir().c_str(), GetLayoutName().c_str());

	if (!serialization::SerializeToFile(layoutPath, m_NodeHierachy))
	{
		LOG(FS("EditorBase::SaveLayout > unable to save the layout to: %s", layoutPath.c_str()), LogLevel::Warning);
	}
}

//---------------------------------
// EditorBase::OnAllocationAvailable
//
// Set the positions of split handles
//
void EditorBase::OnAllocationAvailable()
{
	if (!m_HasInitialSize)
	{
		return;
	}

	if (!m_NodeHierachy.root->IsLeaf())
	{
		static_cast<EditorSplitNode*>(m_NodeHierachy.root)->AdjustLayout();
	}
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
