#include "stdafx.h"
#include "ResourceBrowser.h"

#include <gtkmm/frame.h>


namespace et {
namespace edit {


//==========================
// Resource Browser
//==========================


//------------------------
// ResourceBrowser::c-tor
//
ResourceBrowser::ResourceBrowser()
	: I_EditorTool()
{
}

//-----------------------
// ResourceBrowser::Init
//
// Tool initialization implementation
//
void ResourceBrowser::Init(EditorBase* const editor, Gtk::Frame* parent)
{
	m_View.Init(std::vector<rttr::type>());

	parent->add(*(m_View.GetAttachment()));
	m_View.GetAttachment()->show_all();
}

//------------------------------------
// ResourceBrowser::GetToolbarContent
//
// Remove controls from internal resource view so that it can be hooked up to the toolbar instead
//
Gtk::Widget* ResourceBrowser::GetToolbarContent()
{
	// the toolbar comes embedded in the view.
	Gtk::Box* const toolbar = m_View.GetToolbar();

	// if it is still attached to the parent remove it
	Gtk::Container* const parent = toolbar->get_parent();
	if (parent != nullptr)
	{
		parent->remove(*toolbar);
	}

	// for easier viewing
	toolbar->set_margin_left(20);
	toolbar->set_margin_right(20);

	// pass it to the tool
	return toolbar;
}


} // namespace edit
} // namespace et
