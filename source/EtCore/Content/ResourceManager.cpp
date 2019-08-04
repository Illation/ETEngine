#include "stdafx.h"
#include "ResourceManager.h"

#include <EtCore/Reflection/Serialization.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Package/MemoryPackage.h>
#include <EtCore/FileSystem/Package/FilePackage.h>


//===================
// Resource Manager
//===================


//---------------------------------
// ResourceManager::d-tor
//
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
	std::vector<uint8> rawData;
	if (!memPkg->GetEntryData(GetHash(s_DatabasePath), rawData))
	{
		LOG("ResourceManager::InitFromPackageData > Unable to retrieve database from memory package at '" + std::string(s_DatabasePath) + 
			std::string("'"), LogLevel::Error);
		return;
	}

	// convert that data to a string and deserialize it as json
	if (!serialization::DeserializeFromJsonString(FileUtil::AsText(rawData), m_Database))
	{
		LOG("ResourceManager::InitFromPackageData > unable to deserialize asset database at '" + std::string(s_DatabasePath) + std::string("'"), 
			LogLevel::Error);
	}

	// Create the file packages for all indexed packages
	for (AssetDatabase::PackageDescriptor const& desc : m_Database.packages)
	{
		FilePackage* const filePkg = new FilePackage(desc.GetPath() + desc.GetName() + FilePackage::s_PackageFileExtension);
		m_Packages.emplace_back(desc.GetId(), filePkg);
	}

	// Link asset references together
	for (AssetDatabase::AssetCache& cache : m_Database.caches)
	{
		// every asset per cache
		for (I_Asset* asset : cache.cache)
		{
			for (I_Asset::Reference& ref : asset->m_References)
			{
				ref.Init();
			}
		}
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
// ResourceManager::GetPackage
//
// Get an abstract package by its ID - if none is found returns nullptr
//
I_Package* ResourceManager::GetPackage(T_Hash const id)
{
	// find the package iterator
	auto const foundPackageIt = std::find_if(m_Packages.begin(), m_Packages.end(), [&id](T_IndexedPackage const& indexedPackage)
	{
		return indexedPackage.first == id;
	});

	// check the iterator is valid
	if (foundPackageIt == m_Packages.cend())
	{
		return nullptr;
	}

	return foundPackageIt->second;
}

//---------------------------------
// ResourceManager::GetAsset
//
// Get an asset by its ID
//
I_Asset* ResourceManager::GetAsset(T_Hash const assetId)
{
	// in this version we loop over all caches
	for (AssetDatabase::AssetCache& cache : m_Database.caches)
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
	LOG("ResourceManager::GetAsset > Couldn't find asset with ID '" + std::to_string(assetId) + std::string("'!"), LogLevel::Warning);
	return nullptr;
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
