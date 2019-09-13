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
I_Asset* AssetDatabase::GetAsset(T_Hash const assetId)
{
	// in this version we loop over all caches
	for (AssetCache& cache : caches)
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
	LOG("AssetDatabase::GetAsset > Couldn't find asset with ID '" + std::to_string(assetId) + std::string("'!"), LogLevel::Warning);
	return nullptr;
}

//---------------------------------
// AssetDatabase::GetAsset
//
// Get an asset by its ID and type
//
I_Asset* AssetDatabase::GetAsset(T_Hash const assetId, std::type_info const& type)
{
	// Try finding a cache containing our type
	auto foundCacheIt = std::find_if(caches.begin(), caches.end(), [&type](AssetCache& cache)
	{
		return cache.GetType() == type;
	});

	if (foundCacheIt == caches.cend())
	{
		LOG("AssetDatabase::GetAsset > Couldn't find asset cache of type '" + std::string(type.name()) + std::string("'!"), LogLevel::Warning);
		return nullptr;
	}

	// try finding our asset by its ID in the cache
	auto foundAssetIt = std::find_if(foundCacheIt->cache.begin(), foundCacheIt->cache.end(), [assetId](I_Asset* asset)
	{
		return asset->GetId() == assetId;
	});

	if (foundAssetIt == foundCacheIt->cache.cend())
	{
		LOG("ResourceManager::GetAsset > Couldn't find asset with ID '" + std::to_string(assetId) + std::string("'!"), LogLevel::Warning);
		return nullptr;
	}

	return *foundAssetIt;
}

//---------------------------------
// AssetDatabase::Merge
//
// Merge another asset database into this one. This moves the other database's content into this one, so the other DB will be empty.
//
void AssetDatabase::Merge(AssetDatabase const& other)
{
	// not implemented	
}
