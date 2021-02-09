#pragma once

#include <EtEditor/Content/ResourceView.h>
#include <EtEditor/Layout/EditorTool.h>


namespace et {
namespace edit {


//--------------------
// ResourceBrowser
//
// allows viewing a list of all assets
//
class ResourceBrowser final : public I_EditorTool
{
	// definitions
	//-------------
	static std::string const s_DropFileType;

public:
	// construct destruct
	//--------------------
	ResourceBrowser();
	~ResourceBrowser() = default;

	// Editor tool interface
	//-----------------------
	void Init(EditorBase* const editor, Gtk::Frame* parent) override;
	Gtk::Widget* GetToolbarContent() override;

	// utility
	//---------
protected:
	void OnDropDataReceived(Glib::RefPtr<Gdk::DragContext> const& context, 
		int32 x, 
		int32 y, 
		Gtk::SelectionData const& selectionData, 
		guint info, 
		guint time);


	// Data
	///////

private:
	ResourceView m_View;
};


} // namespace edit
} // namespace et

