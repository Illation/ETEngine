#include "stdafx.h"
#include "ResourceManager.h"

#include "Asset.h"

#include <EtCore/Reflection/Serialization.h>

#include <rttr/registration>

#include <EtCore/FileSystem/FileUtil.h>


//===================
// Resource Manager
//===================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<ResourceManager::AssetDatabase::AssetCache>("asset cache")
		.property("cache", &ResourceManager::AssetDatabase::AssetCache::cache);

	registration::class_<ResourceManager::AssetDatabase>("asset database")
		.property("caches", &ResourceManager::AssetDatabase::caches) ;
}


//---------------------------------
// ResourceManager::AssetDatabase::AssetCache::GetType
//
// Get the type of an asset cache
//
std::type_info const& ResourceManager::AssetDatabase::AssetCache::GetType() const
{
	if (cache.size() > 0)
	{
		return cache[0]->GetType();
	}
	return typeid(nullptr);
}


// Managing assets
////////////////////////

ResourceManager::~ResourceManager()
{
	Deinit();
}

//---------------------------------
// ResourceManager::Init
//
// Load the asset database
//
void ResourceManager::Init()
{
	if (!serialization::DeserializeFromJsonResource(s_DatabasePath, m_Database))
	{
		LOG("ResourceManager::Init > unable to deserialize asset database at '" + std::string(s_DatabasePath) + std::string("'"), LogLevel::Error);
	}
}

//---------------------------------
// ResourceManager::InitFromFile
//
// Load the asset database from a physical JSON file in case it hasn't been embedded into cooked resources yet
//
void ResourceManager::InitFromFile(std::string const& path)
{
	if (!serialization::DeserializeFromFile(path, m_Database))
	{
		LOG("ResourceManager::InitFromFile > unable to deserialize asset database at '" + std::string(path) + std::string("'"), LogLevel::Error);
	}
}

//---------------------------------
// ResourceManager::Deinit
//
// Delete all remaining open assets
//
void ResourceManager::Deinit()
{
	for (AssetDatabase::AssetCache& cache : m_Database.caches)
	{
		for (I_Asset* asset : cache.cache)
		{
			delete asset;
			asset = nullptr;
		}
	}
	m_Database.caches.clear();
}

//---------------------------------
// ResourceManager::GetAsset
//
// Get an asset by its ID and type
//
I_Asset* ResourceManager::GetAsset(T_Hash const assetId, std::type_info const& type)
{
	// Try finding a cache containing our type
	auto foundCacheIt = std::find_if(m_Database.caches.begin(), m_Database.caches.end(), [&type](AssetDatabase::AssetCache& cache)
	{
		return cache.GetType() == type;
	});

	if (foundCacheIt == m_Database.caches.cend())
	{
		LOG("ResourceManager::GetAsset > Couldn't find asset cache of type '" + std::string(type.name()) + std::string("'!"), LogLevel::Warning);
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
