#include "stdafx.h"

#include "Package.h"

#include <EtCore/FileSystem/FileUtil.h>


//=====================
// Memory Package
//=====================


// ctor dtor
//////////////

//---------------------------------
// MemoryPackage::MemoryPackage
//
// Construct a memory package with a pointer to its data, initialize the entry map
//
MemoryPackage::MemoryPackage(uint8 const* const data)
	: m_Data(data)
{
	InitFileListFromData();
}


// Utility
////////////

//---------------------------------
// MemoryPackage::GetEntry
//
// Get a package entry from a package using its hashed ID
//
MemoryPackage::PackageEntry const* MemoryPackage::GetEntry(T_Hash const id) const
{
	auto findResult = m_Entries.find(id);

	// nullptr indicates we didn't find an entry with the ID
	if (findResult == m_Entries.cend())
	{
		return nullptr;
	}

	// pointer to value
	return &(findResult->second);
}

//---------------------------------
// MemoryPackage::GetEntryData
//
// Get the data from a package entry, with size being an out parameter. 
// If no entry was found for the ID, we return nullptr and size is undefined.
//
uint8 const* MemoryPackage::GetEntryData(T_Hash const id, uint64& size)
{
	// try getting the file
	PackageEntry const* pkgEntry = GetEntry(id);
	if (pkgEntry == nullptr)
	{
		return nullptr;
	}

	// return the content of the file
	size = pkgEntry->size;
	return pkgEntry->content;
}

//---------------------------------
// MemoryPackage::InitFileListFromData
//
// Initializes the entry map
//
void MemoryPackage::InitFileListFromData()
{
	// read the package header 
	PkgHeader const* pkgHeader = reinterpret_cast<PkgHeader const*>(m_Data);
	size_t offset = sizeof(PkgHeader);

	// read the central directory
	std::vector<std::pair<T_Hash, uint64>> centralDirectory;
	for (size_t infoIdx = 0u; infoIdx < pkgHeader->numEntries; ++infoIdx)
	{
		PkgFileInfo const* info = reinterpret_cast<PkgFileInfo const*>(m_Data + offset);
		offset += sizeof(PkgFileInfo);

		centralDirectory.emplace_back(info->fileId, info->offset);
	}

	// read the files listed
	for (std::pair<T_Hash, uint64> const& fileInfo : centralDirectory)
	{
		// start at the offset from the beginning of the package
		offset = fileInfo.second;

		PkgEntry const* entry = reinterpret_cast<PkgEntry const*>(m_Data + offset);
		offset += sizeof(PkgEntry);

		// get and validate the fileId
		if (entry->fileId != fileInfo.first)
		{
			LOG("Package::InitFileListFromData > File ID didn't match file info from central directory! Expected [" 
				+ std::to_string(fileInfo.first) + std::string("] - found [") + std::to_string(entry->fileId) + std::string("]"), 
				LogLevel::Warning);
			continue;
		}

		// Create our package file in the map and edit it after to avoid unnecessary file copying
		auto emplaceIt = m_Entries.try_emplace(entry->fileId, PackageEntry());

		if (!emplaceIt.second)
		{
			LOG("Package::InitFileListFromData > Entry list already contains a file with ID [" + std::to_string(entry->fileId) + std::string("] !"),
				LogLevel::Warning);
			continue;
		}

		PackageEntry& pkgEntry = emplaceIt.first->second;

		// read the const size variables from the package file
		pkgEntry.compressionType = entry->compressionType;
		pkgEntry.size = entry->size;

		// read the file name and split 
		std::string fullName;
		for (size_t letterIdx = 0u; letterIdx < entry->nameLength; ++letterIdx)
		{
			fullName += static_cast<char>(m_Data[offset++]);
		}

		pkgEntry.fileName = FileUtil::ExtractName(fullName);
		pkgEntry.path = FileUtil::ExtractPath(fullName);

		// set the pointer to the content
		pkgEntry.content = m_Data + offset;
	}
}
