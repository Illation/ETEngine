#include "stdafx.h"
#include "AssetDatabase.h"

#include <rttr/registration>



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
	m_Id = GetHash(m_Name);
}


//====================
// Asset Cache
//====================


//---------------------------------
// AssetDatabase::AssetCache::GetType
//
// Get the type of an asset cache
//
std::type_info const& AssetDatabase::AssetCache::GetType() const
{
	if (cache.size() > 0)
	{
		return cache[0]->GetType();
	}
	return typeid(nullptr);
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
AssetDatabase::T_AssetList AssetDatabase::GetAssetsInPackage(T_Hash const packageId)
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

//---------------------------------
// AssetDatabase::GetAsset
//
// Get an asset by its ID
//
I_Asset* AssetDatabase::GetAsset(T_Hash const assetId, bool const reportErrors) const
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
		LOG("AssetDatabase::GetAsset > Couldn't find asset with ID '" + std::to_string(assetId) + std::string("'!"), LogLevel::Warning);
	}
	return nullptr;
}

//---------------------------------
// AssetDatabase::GetAsset
//
// Get an asset by its ID and type
//
I_Asset* AssetDatabase::GetAsset(T_Hash const assetId, std::type_info const& type, bool const reportErrors) const
{
	// Try finding a cache containing our type
	auto const foundCacheIt = std::find_if(caches.cbegin(), caches.cend(), [&type](AssetCache const& cache)
	{
		return cache.GetType() == type;
	});

	if (foundCacheIt == caches.cend())
	{
		if (reportErrors)
		{
			LOG("AssetDatabase::GetAsset > Couldn't find asset cache of type '" + std::string(type.name()) + std::string("'!"), LogLevel::Warning);
		}

		return nullptr;
	}

	// try finding our asset by its ID in the cache
	auto const foundAssetIt = std::find_if(foundCacheIt->cache.cbegin(), foundCacheIt->cache.cend(), [assetId](I_Asset const* const asset)
	{
		return asset->GetId() == assetId;
	});

	if (foundAssetIt == foundCacheIt->cache.cend())
	{
		if (reportErrors)
		{
			LOG("ResourceManager::GetAsset > Couldn't find asset with ID '" + std::to_string(assetId) + std::string("'!"), LogLevel::Warning);
		}

		return nullptr;
	}

	return *foundAssetIt;
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
						rhAsset->GetPackageName().c_str());

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
						(*assetIt)->GetPackageName().c_str(),
						rhAsset->GetPackageName().c_str())
						, LogLevel::Error);
				}
			}
		}
	}
}
