#include "stdafx.h"

#include "PackageWriter.h"

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>


//===================================
// Package Writer :: File Entry Info
//===================================


//---------------------------------------------
// PackageWriter::FileEntryInfo::FileEntryInfo
//
// File entry info constructor that assigns all members
//
PackageWriter::FileEntryInfo::FileEntryInfo(PkgEntry const& lEntry, File* const lFile, std::string const& lRelName) 
	: entry(lEntry)
	, file(lFile)
	, relName(lRelName)
{ }


//======================
// Package Writer
//======================


//---------------------------------
// PackageWriter::~PackageWriter
//
// Close all files in destructor
//
PackageWriter::~PackageWriter()
{
	for (FileEntryInfo& entryFile : m_Files)
	{
		if (entryFile.file->IsOpen())
		{
			entryFile.file->Close();

			// delete the file
			delete entryFile.file;
			entryFile.file = nullptr;
		}
	}
}

//---------------------------------
// PackageWriter::AddFile
//
// Add a file to the writer and create a package entry for it - takes ownership
//
void PackageWriter::AddFile(File* const file, std::string const& rootDir, E_CompressionType const compression)
{
	m_Files.emplace_back(PkgEntry(), file, std::string());
	PkgEntry& entry = m_Files[m_Files.size() - 1].entry;
	std::string& relName = m_Files[m_Files.size() - 1].relName;

	// assign the name the relative path of the file compared to the root directory of the package writer
	relName = FileUtil::GetRelativePath(file->GetName(), rootDir);

	entry.fileId = GetHash(relName);
	entry.compressionType = compression; 
	entry.nameLength = static_cast<uint16>(relName.size());

	// try opening the file for now, so that we can get the size, keep it open for copying content later
	if (!file->Open(FILE_ACCESS_MODE::Read))
	{
		LOG("PackageWriter::AddFile > unable to open file '" + relName + std::string("'!"), Warning);
		return;
	}

	// get the file size
	entry.size = file->GetSize();
}

//---------------------------------
// PackageWriter::Write
//
// Write the listed files to the data vector
//
void PackageWriter::Write(std::vector<uint8>& data)
{
	// we do a first pass where we figure out the relative offset for all files
	//---------------------------
	uint64 offset = 0u;

	// as we go we can figure out the header
	PkgHeader header;
	header.numEntries = static_cast<uint64>(m_Files.size());
	offset += static_cast<uint64>(sizeof(PkgHeader));

	// and the size that our central directory will be
	offset += static_cast<uint64>(sizeof(PkgFileInfo)) * header.numEntries;

	// as we go we can fill in the entry information
	std::vector<PkgFileInfo> fileInfos;
	for (FileEntryInfo& entryFile : m_Files)
	{
		PkgFileInfo info;
		info.fileId = entryFile.entry.fileId;
		info.offset = offset;

		// we already know the offset so we can add the file entry
		fileInfos.emplace_back(info);

		// accumulate our offset
		PkgEntry& entry = entryFile.entry;
		offset += sizeof(PkgEntry) + static_cast<uint64>(entry.nameLength) + entry.size;
	}

	// now we know the total size of the package file will be our offset, so we can initialize our data vector starting with the right size
	//---------------------------
	data = std::vector<uint8>(static_cast<size_t>(offset), 0u);
	uint8* raw = data.data();

	// finally we can write our data to the vector
	//---------------------------

	// header
	memcpy(raw, &header, sizeof(PkgHeader));
	offset = sizeof(PkgHeader);	// we can reuse it now that our vector has bin initialized

	// central dir
	for (PkgFileInfo const& info : fileInfos)
	{
		memcpy(raw + offset, &info, sizeof(PkgFileInfo));
		offset += sizeof(PkgFileInfo);
	}

	// files
	for (size_t entryIndex = 0u; entryIndex < m_Files.size(); ++entryIndex)
	{
		FileEntryInfo& entryFile = m_Files[entryIndex];

		if (offset != fileInfos[entryIndex].offset)
		{
			LOG("PackageWriter::Write > Entry offset doesn't match expected offset - " + entryFile.relName, LogLevel::Error);
		}

		// copy the entry
		memcpy(raw + offset, &entryFile.entry, sizeof(PkgEntry));
		offset += sizeof(PkgEntry);

		// copy the file name string
		if (entryFile.entry.nameLength != entryFile.relName.size())
		{
			LOG("PackageWriter::Write > Entry name length doesn't match file name length - " + entryFile.relName, LogLevel::Error);
		}

		memcpy(raw + offset, entryFile.relName.c_str(), entryFile.entry.nameLength);
		offset += entryFile.entry.nameLength;

		// copy the file content
		std::vector<uint8> fileContent = entryFile.file->Read();

		if (entryFile.entry.size != static_cast<uint64>(fileContent.size()))
		{
			LOG("PackageWriter::Write > Entry size doesn't match read file contents size - " + entryFile.relName, LogLevel::Error);
		}

		memcpy(raw + offset, fileContent.data(), entryFile.entry.size);
		offset += entryFile.entry.size;
	}
}
