#include "stdafx.h"
#include "ResourceBrowser.h"

#include <gtkmm/frame.h>

#include <EtCore/FileSystem/FileUtil.h>

#include <EtEditor/Content/Importer.h>


namespace et {
namespace edit {


//==========================
// Resource Browser
//==========================


std::string const ResourceBrowser::s_DropFileType("text/uri-list");
std::string const ResourceBrowser::s_UriFile("file:///");


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

	// Drag to import
	Gtk::Widget* const dragArea = m_View.GetAssetArea();
	dragArea->drag_dest_set({Gtk::TargetEntry(s_DropFileType)}, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
	dragArea->signal_drag_data_received().connect(sigc::mem_fun(*this, &ResourceBrowser::OnDropDataReceived));
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

//------------------------------------
// ResourceBrowser::OnDropDataReceived
//
// Dragging to import assets - in the future perhaps to restructure directories
//
void ResourceBrowser::OnDropDataReceived(Glib::RefPtr<Gdk::DragContext> const& context, 
	int32 x, 
	int32 y, 
	Gtk::SelectionData const& selectionData, 
	guint info, 
	guint time)
{
	bool dropFinished = false;

	if ((selectionData.get_length() >= 0) && (selectionData.get_data_type() == s_DropFileType))
	{
		std::vector<Glib::ustring> const uris = selectionData.get_uris();
		for (Glib::ustring const& uri : uris)
		{
			if (uri.find(s_UriFile) == 0u)
			{
				std::string const path(uri.substr(s_UriFile.size()));
				ImporterBase const* const importer = ImporterBase::GetImporter(path);
				if (importer != nullptr)
				{
					E_ImportResult const res = importer->Run(path);
					switch (res)
					{
					case E_ImportResult::Cancelled:
						LOG(FS("Import cancelled for file '%s'", path.c_str()));
						break;
					case E_ImportResult::Failed:
						LOG(FS("Import failed for file '%s'", path.c_str()));
						break;
					case E_ImportResult::Succeeded:
						LOG(FS("Import succeeded for file '%s'", path.c_str()));
						dropFinished = true;
						break;
					}
				}
				else
				{
					LOG(FS("No valid importer for file '%s' found!", path.c_str()));
				}
			}
			else
			{
				LOG(FS("Unsupported uri type '%s'", uri.c_str()));
			}
		}
	}
	else
	{
		LOG(FS("Unsupported drop data '%s'", selectionData.get_data_as_string().c_str()));
	}

	context->drag_finish(dropFinished, false, time);
}


} // namespace edit
} // namespace et
