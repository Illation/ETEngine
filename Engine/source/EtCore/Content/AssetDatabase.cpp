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

	registration::class_<AssetDatabase::PackageDescriptor>("package")
		.property("name", &AssetDatabase::PackageDescriptor::GetName, &AssetDatabase::PackageDescriptor::SetName)
		.property("path", &AssetDatabase::PackageDescriptor::GetPath, &AssetDatabase::PackageDescriptor::SetPath);

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
// Package Descriptor
//====================


//---------------------------------
// AssetDatabase::PackageDescriptor::SetName
//
// Sets the name of a package generator and generates its ID
//
void AssetDatabase::PackageDescriptor::SetName(std::string const& val)
{
	m_Name = val;
	m_Id = m_Name.c_str();
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
// AssetDatabase::GetAssetsInPackage
//
// finds all assets that belong to a certain package
//
AssetDatabase::T_AssetList AssetDatabase::GetAssetsInPackage(HashString const packageId)
{
	T_AssetList outAssets;

	// caches for every asset type 
	for (AssetDatabase::AssetCache& cache : caches)
	{
		// every asset per cache
		for (I_Asset* asset : cache.cache)
		{
			if (asset->GetPackageId() == packageId)
			{
				outAssets.emplace_back(asset);
			}
		}
	}

	return outAssets;
}

//---------------------------------------
// AssetDatabase::GetAssetsMatchingQuery
//
// finds all assets that are contained in a path
//  - if recursive is enabled assets are also found in subdirectories
//  - if searchTerm isn't an empty string, only assets containing the search term will be returned
//  - if filteredTypes isn't empty, only assets of types contained in filtered types are returned
//
AssetDatabase::T_AssetList AssetDatabase::GetAssetsMatchingQuery(std::string const& path,
	bool const recursive, 
	std::string const& searchTerm,
	std::vector<rttr::type> const& filteredTypes)
{
	T_AssetList outAssets;

	std::string lowerSearch = searchTerm;
	std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

	// caches for every asset type 
	for (AssetDatabase::AssetCache& cache : caches)
	{
		if (filteredTypes.size() > 0u)
		{
			rttr::type const cacheType = cache.GetAssetType();
			if (std::find(filteredTypes.begin(), filteredTypes.end(), cacheType) == filteredTypes.cend())
			{
				continue;
			}
		}

		// every asset per cache
		for (I_Asset* asset : cache.cache)
		{
			if ((asset->GetPath().rfind(path, 0) == 0) && (recursive || (asset->GetPath().length() == path.length())))
			{
				bool matchesSearch = true;
				if (lowerSearch.length() != 0u)
				{
					std::string lowerAsset = asset->GetPath() + asset->GetName();
					std::transform(lowerAsset.begin(), lowerAsset.end(), lowerAsset.begin(), ::tolower);
					matchesSearch = (lowerAsset.find(lowerSearch) != std::string::npos);
				}

				if (matchesSearch)
				{
					outAssets.emplace_back(asset);
				}
			}
		}
	}

	return outAssets;
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

//---------------------------------
// AssetDatabase::Merge
//
// Merge another asset database into this one. 
//
void AssetDatabase::Merge(AssetDatabase const& other)
{
	// add packages
	for (PackageDescriptor const& desc : other.packages)
	{
		auto packageIt = std::find_if(packages.cbegin(), packages.cend(), [&desc](PackageDescriptor const& lhs) 
			{	
				return lhs.GetId() == desc.GetId();
			});

		// if the other DB contains a package that this DB doesn't know of, add it
		if (packageIt == packages.cend())
		{
			packages.emplace_back(desc);
		}
		else
		{
			// if both have the same package, ensure they agree on its details
			ET_ASSERT(packageIt->GetPath() == desc.GetPath(),
				"DBs disagree on paths for package '%s'! this: '%s' - other: '%s'", 
				desc.GetName().c_str(),
				packageIt->GetPath().c_str(),
				desc.GetPath().c_str());
		}
	}

	// add caches
	for (AssetCache const& rhCache : other.caches)
	{
		auto cacheIt = std::find_if(caches.begin(), caches.end(), [&rhCache](AssetCache const& lhCache)
			{
				return lhCache.GetType() == rhCache.GetType();
			});

		if (cacheIt == caches.cend())
		{
			caches.emplace_back(rhCache);
		}
		else
		{
			AssetCache& lhCache = *cacheIt;

			// insert assets
			for (I_Asset* const rhAsset : rhCache.cache)
			{
				// Ensure the asset doesn't already exist
				auto assetIt = std::find_if(lhCache.cache.cbegin(), lhCache.cache.cend(), [rhAsset](I_Asset const* const lhAsset)
				{
					return lhAsset->GetId() == rhAsset->GetId();
				});

				// if the to merge asset is unique add it
				if (assetIt == lhCache.cache.cend())
				{
					// check we have a package descriptor for the new asset
					ET_ASSERT(std::find_if(packages.cbegin(), packages.cend(), [rhAsset](PackageDescriptor const& lhPackage)
						{
							return lhPackage.GetId() == rhAsset->GetPackageId();
						}) != packages.cend() || rhAsset->GetPackageId() == 0u,
						"Asset merged into DB, but DB doesn't contain package '%s'",
						rhAsset->GetPackageId().ToStringDbg());

					lhCache.cache.emplace_back(rhAsset);
				}
				else
				{
					// if the asset is already included, that's an issue
					LOG(FS("AssetDatabase::Merge > Asset already contained in this DB! "
						"Name: '%s', Path: '%s', Merge Path: '%s', Package: '%s', Merge Package: '%s'",
						rhAsset->GetName().c_str(),
						(*assetIt)->GetPath().c_str(),
						rhAsset->GetPath().c_str(),
						(*assetIt)->GetPackageId().ToStringDbg(),
						rhAsset->GetPackageId().ToStringDbg())
						, LogLevel::Error);
				}
			}
		}
	}
}


} // namespace core
} // namespace et
