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
public:
	// construct destruct
	//--------------------
	ResourceBrowser();
	~ResourceBrowser() = default;

	// Editor tool interface
	//-----------------------
	void Init(EditorBase* const editor, Gtk::Frame* parent) override;
	Gtk::Widget* GetToolbarContent() override;


	// Data
	///////

private:
	ResourceView m_View;
};


} // namespace edit
} // namespace et

