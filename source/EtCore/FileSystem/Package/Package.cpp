#include "stdafx.h"

#include "Package.h"

#include <EtCore/FileSystem/FileUtil.h>


//=====================
// Viewport
//=====================


// ctor dtor
//////////////

//---------------------------------
// Package::Package
//
// Construct a package with a pointer to its data, initialize the entry map
//
Package::Package(uint8 const* const data)
	: m_Data(data)
{
	InitFileListFromData();
}


// Utility
////////////

//---------------------------------
// Package::GetFile
//
// Get a package file from a package using its hashed ID
//
Package::PackageFile const* Package::GetFile(T_Hash const id) const
{
	return nullptr;
}

//---------------------------------
// Package::InitFileListFromData
//
// Initializes the entry map
//
void Package::InitFileListFromData()
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
		auto emplaceIt = m_Entries.try_emplace(entry->fileId, PackageFile());

		if (!emplaceIt.second)
		{
			LOG("Package::InitFileListFromData > Entry list already contains a file with ID [" + std::to_string(entry->fileId) + std::string("] !"),
				LogLevel::Warning);
			continue;
		}

		PackageFile& pkgFile = emplaceIt.first->second;

		// read the const size variables from the package file
		pkgFile.compressionType = entry->compressionType;
		pkgFile.size = entry->size;

		// read the file name and split 
		std::string fullName;
		for (size_t letterIdx = 0u; letterIdx < entry->nameLength; ++letterIdx)
		{
			fullName += static_cast<char>(m_Data[offset++]);
		}

		pkgFile.fileName = FileUtil::ExtractName(fullName);
		pkgFile.path = FileUtil::ExtractPath(fullName);

		// set the pointer to the content
		pkgFile.content = m_Data + offset;
	}
}
