#include "stdafx.h"
#include "AssetTypeFilter.h"

#include <glibmm/main.h>
#include <glibmm/object.h>
#include <gtkmm/menu.h>


namespace et {
namespace edit {


//===================
// Asset Type Filter
//===================


//------------------------
// AssetTypeFilter::Init
//
void AssetTypeFilter::Init(Gtk::MenuButton* const button)
{
	ET_ASSERT(button != nullptr);
	m_MenuButton = button;

	m_RefBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/menu_assetTypes.ui");

	Glib::RefPtr<Glib::Object> object = m_RefBuilder->get_object("assettypemenu");
	m_GMenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);

	//for ()
	//{
	//	m_Types.push_back(AssetType());
	//	AssetType& assetType = m_Types.back();

	//	assetType.m_Item = Gio::MenuItem::create("_Flip to top", "assettype.flip");
	//	m_GMenu->prepend_item(assetType.m_Item);
	//}

	m_Menu = new Gtk::Menu(m_GMenu);

	// General functionality
	//------------------------
	m_OtherActionGroup = Gio::SimpleActionGroup::create();

	auto onRecursive = [this]()
	{
		m_RecursiveDirectories = !m_RecursiveDirectories;
		m_OtherActionGroup->lookup("recursive")->change_state(m_RecursiveDirectories);
		NotifyOnChange();
	};
	m_OtherActionGroup->add_action_bool("recursive", onRecursive, m_RecursiveDirectories);

	m_Menu->insert_action_group("other", m_OtherActionGroup);

	// type functionality
	//------------------------
	m_TypeActionGroup = Gio::SimpleActionGroup::create();

	m_Menu->insert_action_group("type", m_TypeActionGroup);
	m_MenuButton->set_menu(*m_Menu);
}

//-----------------------------------
// AssetTypeFilter::RegisterListener
//
void AssetTypeFilter::RegisterListener(I_Listener* const listener)
{
	ET_ASSERT(std::find(m_Listeners.cbegin(), m_Listeners.cend(), listener) == m_Listeners.cend());
	m_Listeners.push_back(listener);
}

//-------------------------------------
// AssetTypeFilter::UnregisterListener
//
void AssetTypeFilter::UnregisterListener(I_Listener const* const listener)
{
	auto const foundIt = std::find(m_Listeners.begin(), m_Listeners.end(), listener);
	ET_ASSERT(foundIt != m_Listeners.cend());
	
	m_Listeners.erase(foundIt);
}

//---------------------------------
// AssetTypeFilter::NotifyOnChange
//
void AssetTypeFilter::NotifyOnChange()
{
	for (I_Listener* const listener : m_Listeners)
	{
		listener->OnAssetTypeFilterChanged();
	}
}


} // namespace edit
} // namespace et

