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
			for (I_Asset::Reference& reference : asset->m_References)
			{
				reference.m_Asset = m_Database.GetAsset(GetHash(reference.m_Name));
			}
		}
	}
}

//---------------------------------
// ResourceManager::Deinit
//
// Cleans up packages
//
void ResourceManager::Deinit()
{
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
// ResourceManager::Flush
//
// Force unloading all assets with no references
//
void ResourceManager::Flush()
{
	for (AssetDatabase::AssetCache& cache : m_Database.caches)
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
