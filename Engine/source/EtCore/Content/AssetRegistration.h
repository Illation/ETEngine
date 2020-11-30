#pragma once
#include <EtCore/Content/ResourceManager.h>

// utility macros that allow correct serialization of asset pointers

#define REGISTER_ASSET_CONVERSION(TClass)																										\
	rttr::type::register_converter_func([](std::string const& id, bool& ok) -> et::AssetPtr<TClass>												\
	{																																			\
		et::AssetPtr<TClass> const retVal = et::core::ResourceManager::Instance()->GetAssetData<TClass>(et::core::HashString(id.c_str()));		\
		ok = (retVal != nullptr);																												\
		return retVal;																															\
	})

// can't have AssetType meta id as a static const because it is not guaranteed that it will be initialized before everything else is registered
#define BEGIN_REGISTER_CLASS_ASSET(TClass, TName)																					\
	rttr::registration::class_<et::AssetPtr<TClass>>( TName " asset pointer" )(rttr::metadata(et::GetHash("AssetType"), true))		\
	END_REGISTER_CLASS_POLYMORPHIC(et::AssetPtr<TClass>, et::I_AssetPtr);															\
	REGISTER_ASSET_CONVERSION(TClass);																								\
	rttr::registration::class_<TClass>( TName ).constructor<TClass const&>().constructor<>()(rttr::detail::as_object())



