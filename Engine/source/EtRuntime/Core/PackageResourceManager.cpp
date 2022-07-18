#include "stdafx.h"
#include "PackageResourceManager.h"

#include <EtCore/Reflection/BinaryDeserializer.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Package/MemoryPackage.h>
#include <EtCore/FileSystem/Package/FilePackage.h>


namespace et {
namespace rt {


//==========================
// Package Resource Manager
//==========================


//---------------------------------
// PackageResourceManager::c-tor
//
PackageResourceManager::PackageResourceManager() 
	: ResourceManager()
{ }

//---------------------------------
// PackageResourceManager::Init
//
// Load the asset database from raw package data, assuming the raw package contains an entry with an id from s_DatabasePath
//
void PackageResourceManager::Init()
{
	// Create a new memory package from the data
	m_RootPackage = new core::MemoryPackage(core::FileUtil::GetCompiledData());
	m_Packages.emplace_back(0u, m_RootPackage);

	// get the raw json string for the asset database from that package
	std::vector<uint8> rawData;
	if (!m_RootPackage->GetEntryData(core::HashString(s_DatabasePath), rawData))
	{
		ET_LOG_E(ET_CTX_RUNTIME, "PackageResourceManager::Init > Unable to retrieve database from memory package at '%s'", s_DatabasePath);
		return;
	}

	// convert that data to a string and deserialize it as json
	core::BinaryDeserializer deserializer;
	if (!deserializer.DeserializeFromData(rawData, m_Database))
	{
		ET_LOG_E(ET_CTX_RUNTIME, "PackageResourceManager::Init > unable to deserialize asset database at '%s'", s_DatabasePath);
	}

	// Create the file packages for all indexed packages
	for (core::PackageDescriptor const& desc : m_Database.packages)
	{
		core::FilePackage* const filePkg = new core::FilePackage(desc.GetPath() + desc.GetName() + core::FilePackage::s_PackageFileExtension);
		m_Packages.emplace_back(desc.GetId(), filePkg);
	}

	// Link asset references together
	SetAssetReferences(&m_Database, [this](core::HashString const assetId) { return m_Database.GetAsset(assetId); });
}

//---------------------------------
// PackageResourceManager::Deinit
//
// Cleans up packages
//
void PackageResourceManager::Deinit()
{
	// clear the package list
	for (std::pair<core::HashString, core::I_Package* >& package : m_Packages)
	{
		delete package.second;
		package.second = nullptr;
	}

	m_Packages.clear();
}

//--------------------------------------
// PackageResourceManager::GetLoadData
//
// Retrieve the data for this asset from its package
//
bool PackageResourceManager::GetLoadData(core::I_Asset const* const asset, std::vector<uint8>& outData) const
{
	// Get the package the asset lives in
	auto const foundPackageIt = std::find_if(m_Packages.begin(), m_Packages.end(), [asset](T_IndexedPackage const& indexedPackage)
	{
		return indexedPackage.first == asset->GetPackageId();
	});

	// check the iterator is valid
	if (foundPackageIt == m_Packages.cend())
	{
		ET_TRACE_W(ET_CTX_RUNTIME, "No package (id:'%s') found for asset '%s'", asset->GetPackageId().ToStringDbg(), asset->GetName().c_str());
		return false;
	}

	// get binary data from the package
	if (!(foundPackageIt->second->GetEntryData(asset->GetPackageEntryId(), outData)))
	{
		return false;
	}

	return true;
}

//---------------------------------
// PackageResourceManager::Flush
//
void PackageResourceManager::Flush()
{
	m_Database.Flush();
}

//-------------------------------------------
// PackageResourceManager::GetAssetInternal
//
// Get an asset by it's template type
//
core::I_Asset* PackageResourceManager::GetAssetInternal(core::HashString const assetId, rttr::type const type, bool const reportErrors)
{
	return m_Database.GetAsset(assetId, type, reportErrors);
}


} // namespace rt
} // namespace et
