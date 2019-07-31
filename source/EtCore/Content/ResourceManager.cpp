#include "stdafx.h"
#include "ResourceManager.h"

#include <EtCore/Reflection/Serialization.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Package/Package.h>


//===================
// Resource Manager
//===================


ResourceManager::~ResourceManager()
{
	Deinit();
}

//---------------------------------
// ResourceManager::InitFromCompiledData
//
// Load the asset database from raw package data, assuming the raw package contains an entry with an id from s_DatabasePath
//
void ResourceManager::InitFromCompiledData()
{
	// Create a new memory package from the data
	MemoryPackage* const memPkg = new MemoryPackage(FileUtil::GetCompiledData());
	m_Packages.emplace_back(0u, memPkg);

	// get the raw json string for the asset database from that package
	uint64 dataSize = 0u;
	uint8 const* rawJsonDbBytes = memPkg->GetEntryData(GetHash(s_DatabasePath), dataSize);
	if (rawJsonDbBytes == nullptr)
	{
		LOG("ResourceManager::InitFromPackageData > unable to retrieve database from memory package at '" + std::string(s_DatabasePath) + 
			std::string("'", LogLevel::Error));
	}
	std::string jsonDbString;
	FileUtil::AsText(rawJsonDbBytes, dataSize, jsonDbString);

	if (!serialization::DeserializeFromJsonString(jsonDbString, m_Database))
	{
		LOG("ResourceManager::InitFromPackageData > unable to deserialize asset database at '" + std::string(s_DatabasePath) + std::string("'"), 
			LogLevel::Error);
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
	// clear the database
	for (AssetDatabase::AssetCache& cache : m_Database.caches)
	{
		for (I_Asset* asset : cache.cache)
		{
			delete asset;
			asset = nullptr;
		}
	}
	m_Database.caches.clear();

	// clear the package list
	for (std::pair<T_Hash, I_Package* >& package : m_Packages)
	{
		delete package.second;
		package.second = nullptr;
	}
	m_Packages.clear();
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
