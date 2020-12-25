#include "stdafx.h"
#include "AssetDatabase.h"

#include <rttr/registration>

#include "AssetRegistration.h"


namespace et {
namespace core {


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<AssetDatabase::AssetCache>("asset cache")
		.property("cache", &AssetDatabase::AssetCache::cache);

	registration::class_<AssetDatabase>("asset database")
		.property("packages", &AssetDatabase::packages)
		.property("caches", &AssetDatabase::caches);
}


//-----------------------------------
// AssetDatabase::GetValidAssetTypes
//
// Get all asset types that derive from the given type
//
std::vector<rttr::type> AssetDatabase::GetValidAssetTypes(rttr::type const type, bool const reportErrors)
{
	std::vector<rttr::type> assetTypes;
	if (!IsAssetDataType(type))
	{
		rttr::array_range<rttr::type> derivedTypes = type.get_derived_classes();
		for (rttr::type const derivedType : derivedTypes)
		{
			if (IsAssetDataType(derivedType)) // only leaf asset types
			{
				assetTypes.push_back(derivedType);
			}
		}

		if (reportErrors)
		{
			ET_ASSERT(!assetTypes.empty(), "Couldn't find asset type derived from '%s'!", type.get_name().data());
		}
	}
	else
	{
		assetTypes.push_back(type);
	}

	return assetTypes;
}


//====================
// Asset Cache
//====================


//---------------------------------
// AssetDatabase::AssetCache::GetType
//
// Get the type of an asset cache
//
rttr::type AssetDatabase::AssetCache::GetType() const
{
	if (cache.size() > 0)
	{
		return cache[0]->GetType();
	}

	return rttr::type::get(nullptr);
}

//--------------------------------------
// AssetDatabase::AssetCache::GetAssetType
//
// Get the asset type of an asset cache
//
rttr::type AssetDatabase::AssetCache::GetAssetType() const
{
	if (cache.size() > 0)
	{
		return rttr::type::get(*(cache[0]));
	}

	return rttr::type::get(nullptr);
}

//===================
// Asset Database
//===================


//---------------------------------
// AssetDatabase::d-tor
//
// Delete all asset pointers
//
AssetDatabase::~AssetDatabase()
{
	if (!m_OwnsAssets)
	{
		return;
	}

	for (AssetCache& cache : caches)
	{
		for (I_Asset* asset : cache.cache)
		{
			delete asset;
			asset = nullptr;
		}
	}
	caches.clear();
}

//---------------------------------
// AssetDatabase::GetAsset
//
// Get an asset by its ID
//
I_Asset* AssetDatabase::GetAsset(HashString const assetId, bool const reportErrors) const
{
	// in this version we loop over all caches
	for (AssetCache const& cache : caches)
	{
		// try finding our asset by its ID in the cache
		auto foundAssetIt = std::find_if(cache.cache.begin(), cache.cache.end(), [assetId](I_Asset* asset)
		{
			return asset->GetId() == assetId;
		});

		if (foundAssetIt != cache.cache.cend())
		{
			return *foundAssetIt;
		}
	}

	// didn't find an asset in any cache, return null
	if (reportErrors)
	{
		ET_ASSERT(false, "Couldn't find asset with ID '%s'!", assetId.ToStringDbg());
	}

	return nullptr;
}

//---------------------------------
// AssetDatabase::GetAsset
//
// Get an asset by its ID and type
//
I_Asset* AssetDatabase::GetAsset(HashString const assetId, rttr::type const type, bool const reportErrors) const
{
	std::vector<rttr::type> const assetTypes = GetValidAssetTypes(type, reportErrors);

	for (rttr::type const assetType : assetTypes)
	{
		// Try finding a cache containing our type
		auto const foundCacheIt = std::find_if(caches.cbegin(), caches.cend(), [assetType](AssetCache const& cache)
			{
				return cache.GetType() == assetType;
			});

		if (foundCacheIt != caches.cend())
		{
			// try finding our asset by its ID in the cache
			auto const foundAssetIt = std::find_if(foundCacheIt->cache.cbegin(), foundCacheIt->cache.cend(), [assetId](I_Asset const* const asset)
				{
					return asset->GetId() == assetId;
				});

			if (foundAssetIt != foundCacheIt->cache.cend())
			{
				return *foundAssetIt;
			}
		}
	}

	if (reportErrors)
	{
		ET_ASSERT(false, "Couldn't find asset with ID '%s'!", assetId.ToStringDbg());
	}

	return nullptr;
}

//---------------------------------
// AssetDatabase::IterateAllAssets
//
void AssetDatabase::IterateAllAssets(I_AssetDatabase::T_AssetFunc const& func)
{
	for (AssetCache& cache : caches)
	{
		for (I_Asset* asset : cache.cache)
		{
			func(asset);
		}
	}
}

//---------------------------------
// AssetDatabase::Flush
//
// Force unloading all assets with no references
//
void AssetDatabase::Flush()
{
	for (AssetDatabase::AssetCache& cache : caches)
	{
		for (I_Asset* asset : cache.cache)
		{
			if (asset->GetRefCount() <= 0u && asset->IsLoaded())
			{
				asset->Unload(true);
			}
		}
	}
}


} // namespace core
} // namespace et
