#include "stdafx.h"
#include "EditorBase.h"

#include "EditorSplitNode.h"

#include <gtkmm/frame.h>
#include <glibmm/main.h>

#include <EtCore/Reflection/Serialization.h>

#include <EtEditor/Util/GtkUtil.h>
#include <EtEditor/Util/EditorConfig.h>


namespace et {
namespace edit {


//===============
// Editor Base
//===============


//---------------------------------
// EditorBase::OnKeyEvent
//
// by default editors don't handle key events, but this can be overridden by implementations
//
bool EditorBase::OnKeyEvent(bool const pressed, GdkEventKey* const evnt)
{
	ET_UNUSED(pressed);
	ET_UNUSED(evnt);

	return false;
}

//---------------------------------
// EditorBase::Init
//
Gtk::Frame* EditorBase::GetRoot()
{
	return m_NodeHierachy.m_Root->GetAttachment();
}

//---------------------------------
// EditorBase::Init
//
// deserialize the default tool layout and construct the tools from it, then init the editor systems
//
void EditorBase::Init(Gtk::Frame* const parent)
{
	// load the tool hierachy from a layout file
	std::string const layoutPath = FS("%slayouts/%s.json", EditorConfig::GetInstance()->GetEditorUserDir().c_str(), GetLayoutName().c_str());

	if (!(core::serialization::DeserializeFromFile(layoutPath, m_NodeHierachy)))
	{
		LOG(FS("Failed to deserialize layout file for '%s' at: %s", GetName().c_str(), layoutPath.c_str()), core::LogLevel::Warning);
	}

	ET_ASSERT(m_NodeHierachy.m_Root != nullptr);

	// initialize the tools and gtk widgets based on the loaded hierachy
	m_NodeHierachy.m_Root->Init(this, parent, nullptr, m_NodeHierachy.m_Root);

	// once the widget sizes are available, adjust the sizes of widgets based on what the layout loaded
	auto allocateCallback = [this](Gtk::Allocation& allocation)
	{
		ET_UNUSED(allocation);

		m_HasInitialSize = true;
	};
	parent->signal_size_allocate().connect(allocateCallback, true);

	// periodically check if the layout needs to change
	auto onTimeout = [this]() -> bool
	{
		ProcessLayoutChanges();

		return true;
	};
	Glib::signal_timeout().connect(onTimeout, 33); 

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

	if (!core::serialization::SerializeToFile(layoutPath, m_NodeHierachy, true))
	{
		LOG(FS("EditorBase::SaveLayout > unable to save the layout to: %s", layoutPath.c_str()), core::LogLevel::Warning);
	}
}

//---------------------------------
// EditorBase::QueueNodeForSplit
//
void EditorBase::QueueNodeForSplit(WeakPtr<EditorToolNode> const& node)
{
	m_QueuedSplits.emplace_back(node);
}

//---------------------------------
// EditorBase::QueueNodeForCollapse
//
void EditorBase::QueueNodeForCollapse(WeakPtr<EditorToolNode> const& node)
{
	m_QueuedCollapse.emplace_back(node);
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

	if (!m_NodeHierachy.m_Root->IsLeaf())
	{
		RefPtr<EditorSplitNode>::StaticCast(m_NodeHierachy.m_Root)->AdjustLayout();
	}
}

//---------------------------------
// EditorBase::ProcessLayoutChanges
//
// Split or collapse queued nodes based on their state
//
void EditorBase::ProcessLayoutChanges()
{
	for (WeakPtr<EditorToolNode> const& node : m_QueuedSplits)
	{
		m_NodeHierachy.SplitNode(node, this);
	}

	m_QueuedSplits.clear();

	for (WeakPtr<EditorToolNode> const& node : m_QueuedCollapse)
	{
		m_NodeHierachy.CollapseNode(node, this);
	}

	m_QueuedCollapse.clear();
}


} // namespace edit
} // namespace et
