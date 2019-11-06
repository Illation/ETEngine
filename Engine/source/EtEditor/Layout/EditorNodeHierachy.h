#pragma once
#include <giomm/menu.h>
#include <giomm/menuitem.h>

#include <gtkmm/builder.h>
#include <glibmm/refptr.h>


// forward
namespace Gtk {
	class Menu;
}

class EditorBase;
class EditorNode;
class EditorToolNode;


//---------------------------------
// EditorNodeHierachy
//
// Contains for a tree structure of dynamically laid out tools
//
class EditorNodeHierachy
{
	// definitions
	//-------------
	RTTR_ENABLE()

	// construct destruct
	//--------------------
public:
	EditorNodeHierachy();

	// functionality
	//----------------
	void SetHeaderMenuFlipTarget(bool const top);
	void SplitNode(EditorToolNode* const node, EditorBase* const editor);
	void CollapseNode(EditorToolNode* const node, EditorBase* const editor);

	// accessors
	//-----------
	Gtk::Menu* GetHeaderMenu() const { return m_HeaderMenu; }

	// Data
	///////

	EditorNode* root = nullptr;

private:
	Glib::RefPtr<Gtk::Builder> m_RefBuilder;
	Gtk::Menu* m_HeaderMenu = nullptr;
	Glib::RefPtr<Gio::Menu> m_GMenu;
	Glib::RefPtr<Gio::MenuItem> m_FlipItem;
};

