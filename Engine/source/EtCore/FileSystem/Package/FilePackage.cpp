#include "stdafx.h"

#include "FilePackage.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>


namespace et {
namespace core {


//=====================
// File Package
//=====================


// statics
std::string const FilePackage::s_PackageFileExtension(".etpak");


// ctor dtor
//////////////

//---------------------------------
// FilePackage::FilePackage
//
// Construct a file package from its file, initialize the entry map
//
FilePackage::FilePackage(std::string const& path)
{
	m_File = new File(path, nullptr);

	// we just keep package files open for the runtime of the engine, nobody should be writing to them while it's running
	FILE_ACCESS_FLAGS openFlags;
	openFlags.SetFlags(FILE_ACCESS_FLAGS::FLAGS::Exists);
	if (!m_File->Open(FILE_ACCESS_MODE::Read, openFlags))
	{
		LOG("FilePackage::FilePackage > unable to open file '" + path + std::string("'!"), LogLevel::Error);
		return;
	}

	LoadFileList();
}

//---------------------------------
// FilePackage::~FilePackage
//
// Destructor closes the file and deletes the object
//
FilePackage::~FilePackage()
{
	SafeDelete(m_File);
}


// Utility
////////////

//---------------------------------
// FilePackage::GetEntry
//
// Get a package entry from a package using its hashed ID
//
FilePackage::PackageEntry const* FilePackage::GetEntry(HashString const id) const
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
// FilePackage::GetEntryData
//
// This will do a file read from disk
//
bool FilePackage::GetEntryData(HashString const id, std::vector<uint8>& outData)
{
	// try getting the entry
	PackageEntry const* pkgEntry = GetEntry(id);
	if (pkgEntry == nullptr)
	{
		return false;
	}

	outData = std::move(m_File->ReadChunk(pkgEntry->offset, pkgEntry->size));
	return true;
}

//---------------------------------
// FilePackage::LoadFileList
//
// Load all the relevant data about the files without actually loading their content
//
void FilePackage::LoadFileList()
{
	ET_ASSERT(m_File != nullptr);

	// read the package header 
	//-------------------------
	uint64 offset = 0u;
	uint64 nextChunkSize = static_cast<uint64>(sizeof(PkgHeader));
	std::vector<uint8> readBinData(std::move(m_File->ReadChunk(offset, nextChunkSize)));
	offset += nextChunkSize;
	PkgHeader const pkgHeader(*reinterpret_cast<PkgHeader const*>(readBinData.data()));

	// read the central directory
	//----------------------------

	// read the entire data chunk for the central directory from the file
	nextChunkSize = static_cast<uint64>(sizeof(PkgFileInfo)) * pkgHeader.numEntries;
	readBinData = std::move(m_File->ReadChunk(offset, nextChunkSize));

	// map into that read data
	offset = 0u;
	std::vector<PkgFileInfo> centralDirectory;
	for (size_t infoIdx = 0u; infoIdx < pkgHeader.numEntries; ++infoIdx)
	{
		PkgFileInfo const* info = reinterpret_cast<PkgFileInfo const*>(readBinData.data() + static_cast<size_t>(offset));
		offset += static_cast<uint64>(sizeof(PkgFileInfo));

		centralDirectory.emplace_back(*info);
	}

	// read the files listed
	//----------------------------
	for (PkgFileInfo const& fileInfo : centralDirectory)
	{
		// start at the offset from the beginning of the package
		offset = fileInfo.offset;
		nextChunkSize = static_cast<uint64>(sizeof(PkgEntry));
		readBinData = std::move(m_File->ReadChunk(offset, nextChunkSize));
		offset += nextChunkSize;

		PkgEntry const* entry = reinterpret_cast<PkgEntry const*>(readBinData.data());

		// get and validate the fileId

		ET_ASSERT(entry->fileId == fileInfo.fileId,
			"File ID didn't match file info from central directory! Expected [%x] - found [%x]",
			fileInfo.fileId, entry->fileId);

		// Create our package file in the map and edit it after to avoid unnecessary file copying
		auto emplaceIt = m_Entries.try_emplace(entry->fileId, PackageEntry());

		if (!emplaceIt.second)
		{
			ET_ASSERT(false, "Entry list already contains a file with ID [%s] !", entry->fileId.ToStringDbg());
			continue;
		}

		PackageEntry& pkgEntry = emplaceIt.first->second;

		// read the const size variables from the package file
		pkgEntry.compressionType = entry->compressionType;
		pkgEntry.size = entry->size;

		// read the file name and split 
		nextChunkSize = static_cast<uint64>(entry->nameLength);
		std::string fullName = FileUtil::AsText(m_File->ReadChunk(offset, nextChunkSize));

		pkgEntry.fileName = FileUtil::ExtractName(fullName);
		pkgEntry.path = FileUtil::ExtractPath(fullName);

		// set the pointer to the content
		pkgEntry.offset = offset + nextChunkSize;
	}
}


} // namespace core
} // namespace et
