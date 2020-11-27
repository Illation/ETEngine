#include "stdafx.h"
#include "AssetTypeFilter.h"

#include <glibmm/main.h>
#include <glibmm/object.h>
#include <gtkmm/menu.h>

#include <EtCore/Content/Asset.h>


namespace et {
namespace edit {


//===================
// Asset Type Filter
//===================


//------------------------
// AssetTypeFilter::Init
//
// if allowed types is empty every asset type is selectable
//
void AssetTypeFilter::Init(Gtk::MenuButton* const button, std::vector<rttr::type> const& allowedTypes)
{
	m_AllAllowed = allowedTypes.empty();

	// create menu
	//-------------
	ET_ASSERT(button != nullptr);
	m_MenuButton = button;

	m_RefBuilder = Gtk::Builder::create_from_resource("/com/leah-lindner/editor/ui/menu_assetTypes.ui");

	Glib::RefPtr<Glib::Object> object = m_RefBuilder->get_object("assettypemenu");
	m_GMenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);

	m_Menu = new Gtk::Menu(m_GMenu);

	// type functionality
	//------------------------
	m_TypeActionGroup = Gio::SimpleActionGroup::create();

	rttr::type const baseAssetType = rttr::type::get<core::I_Asset>();
	rttr::array_range<rttr::type> derivedTypes = baseAssetType.get_derived_classes();

	Glib::RefPtr<Glib::Object> object2 = m_RefBuilder->get_object("typesection");
	m_TypeSection = Glib::RefPtr<Gio::Menu>::cast_dynamic(object2);

	for (rttr::type const derivedType : derivedTypes)
	{
		if (derivedType.get_derived_classes().size() != 0) // only leaf asset types
		{
			continue;
		}

		bool const isAllowed = (m_AllAllowed || (std::find(allowedTypes.cbegin(), allowedTypes.cend(), derivedType) != allowedTypes.cend()));

		std::string typeName(derivedType.get_name().data());
		std::string actionName(typeName);
		std::transform(actionName.begin(), actionName.end(), actionName.begin(), [](char const ch) 
			{
				return ch == ' ' ? '_' : ch;
			});

		size_t const lastSpace = typeName.rfind(' ');
		if ((lastSpace != std::string::npos) && (typeName.substr(lastSpace + 1) == "asset"))
		{
			typeName = typeName.substr(0, lastSpace);
		}

		m_Types.push_back(AssetType());
		AssetType& assetType = m_Types.back();

		assetType.m_Type = derivedType;
		assetType.m_Item = Gio::MenuItem::create(typeName.c_str(), (std::string("type.") + actionName).c_str());
		assetType.m_IsSelected = (isAllowed && !m_AllAllowed);

		m_TypeSection->append_item(assetType.m_Item);

		auto onType = [this, derivedType, actionName]()
		{
			auto foundIt = std::find_if(m_Types.begin(), m_Types.end(), [derivedType](AssetType const& el)
				{
					return (el.m_Type == derivedType);
				});
			ET_ASSERT(foundIt != m_Types.cend());

			(*foundIt).m_IsSelected = !(*foundIt).m_IsSelected;
			m_TypeActionGroup->lookup(actionName.c_str())->change_state((*foundIt).m_IsSelected);
			NotifyOnChange();
		};
		m_TypeActionGroup->add_action_bool(actionName.c_str(), onType, assetType.m_IsSelected);

		if (!isAllowed)
		{
			Glib::RefPtr<Gio::SimpleAction> action = Glib::RefPtr<Gio::SimpleAction>::cast_static(m_TypeActionGroup->lookup(actionName.c_str()));
			action->set_enabled(false);
		}
	}

	m_GMenu->append_section("Asset Types", m_TypeSection);

	m_Menu->insert_action_group("type", m_TypeActionGroup);

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

	// link with button
	//-------------------
	m_MenuButton->set_menu(*m_Menu);

	// ensure initial type list is correct
	CalculateSelectedTypes();
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
	CalculateSelectedTypes();

	for (I_Listener* const listener : m_Listeners)
	{
		listener->OnAssetTypeFilterChanged();
	}
}

//-----------------------------------------
// AssetTypeFilter::CalculateSelectedTypes
//
void AssetTypeFilter::CalculateSelectedTypes()
{
	m_SelectedTypes.clear();
	for (AssetType const& assetType : m_Types)
	{
		if (assetType.m_IsSelected)
		{
			m_SelectedTypes.push_back(assetType.m_Type);
		}
	}
}


} // namespace edit
} // namespace et

