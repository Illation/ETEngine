#pragma once

#include <glibmm/refptr.h>

#include <giomm/menu.h>
#include <giomm/menuitem.h>
#include <giomm/simpleactiongroup.h>

#include <gtkmm/builder.h>
#include <gtkmm/menubutton.h>


// forward
namespace Gtk {
	class Menu;
}


namespace et {
namespace edit {


//---------------------------------------
// AssetTypeFilter
//
// Menu that lets you select asset types
//
class AssetTypeFilter final
{
	// definitions
	//-------------
	struct AssetType final
	{
		AssetType() = default;

		rttr::type m_Type = rttr::type::get<std::nullptr_t>();
		Glib::RefPtr<Gio::MenuItem> m_Item;
		bool m_IsSelected = false;
	};
public:

	//--------------
	// I_Listener
	//
	// Interface for listening to change events
	//
	class I_Listener
	{
	public:
		virtual ~I_Listener() = default;

		virtual void OnAssetTypeFilterChanged() = 0;
	};

	// construct destruct
	//--------------------
	AssetTypeFilter() = default;

	void Init(Gtk::MenuButton* const button);

	// functionality
	//---------------
	void RegisterListener(I_Listener* const listener);
	void UnregisterListener(I_Listener const* const listener);

	// accessors
	//-----------
	bool AreDirectoriesRecursive() const { return m_RecursiveDirectories; }
	std::vector<rttr::type> const& GetFilteredTypes() const { return m_SelectedTypes; }

	// utility
	//---------
private:
	void NotifyOnChange();


	// Data
	///////

	bool m_RecursiveDirectories = true;
	std::vector<rttr::type> m_SelectedTypes;

	std::vector<I_Listener*> m_Listeners;

	// ui
	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

	Gtk::Menu* m_Menu = nullptr;
	Gtk::MenuButton* m_MenuButton = nullptr;
	Glib::RefPtr<Gio::Menu> m_GMenu;
	Glib::RefPtr<Gio::Menu> m_TypeSection;
	std::vector<AssetType> m_Types;

	Glib::RefPtr<Gio::SimpleActionGroup> m_TypeActionGroup;
	Glib::RefPtr<Gio::SimpleActionGroup> m_OtherActionGroup;
};


} // namespace edit
} // namespace et


