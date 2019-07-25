#include "stdafx.h"

#include "PackageWriter.h"

#include "PackageDataStructure.h"

#include <EtCore/FileSystem/Entry.h>


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
	for (File* file : m_Files)
	{
		if (file->IsOpen())
		{
			file->Close();
		}
	}
}

//---------------------------------
// PackageWriter::AddDirectory
//
// Add all files in a directory to the file list
//
void PackageWriter::AddDirectory(Directory* const directory)
{
	directory->Mount();
	directory->GetChildrenRecursive(m_Files);
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
	std::vector<std::pair<uint64, PkgEntry>> entryOffsets;
	for (File* file : m_Files)
	{
		// we already know the offset so we can add the file entry
		entryOffsets.emplace_back(offset, PkgEntry());
		PkgEntry& entry = entryOffsets[entryOffsets.size() - 1].second;

		// get the name
		std::string fullName = file->GetName();

		entry.fileId = GetHash(fullName);
		entry.compressionType = E_CompressionType::Store; // for now we just store all files
		entry.nameLength = static_cast<uint16>(fullName.size());

		// try opening the file for now, so that we can get the size, keep it open for copying content later
		if (!file->Open(FILE_ACCESS_MODE::Read))
		{
			LOG("PackageWriter::Write > unable to open file '" + fullName + std::string("'!"), Warning);
			continue;
		}

		// get the file size
		entry.size = file->GetSize();

		// accumulate our offset
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
	for (std::pair<uint64, PkgEntry> const& entryOffset : entryOffsets)
	{
		PkgFileInfo info;
		info.fileId = entryOffset.second.fileId;
		info.offset = entryOffset.first;

		memcpy(raw + offset, &info, sizeof(PkgFileInfo));
		offset += sizeof(PkgFileInfo);
	}

	// files
	for (size_t entryIndex = 0u; entryIndex < m_Files.size(); ++entryIndex)
	{
		PkgEntry const& entry = entryOffsets[entryIndex].second;
		File* file = m_Files[entryIndex];
		std::string fullName = file->GetName();

		if (offset != entryOffsets[entryIndex].first)
		{
			LOG("PackageWriter::Write > Entry offset doesn't match expected offset - " + fullName, LogLevel::Error);
		}

		// copy the entry
		memcpy(raw + offset, &entry, sizeof(PkgEntry));
		offset += sizeof(PkgEntry);

		// copy the file name string
		if (entry.nameLength != fullName.size())
		{
			LOG("PackageWriter::Write > Entry name length doesn't match file name length - " + fullName, LogLevel::Error);
		}

		memcpy(raw + offset, fullName.c_str(), entry.nameLength);
		offset += entry.nameLength;

		// copy the file content
		std::vector<uint8> fileContent = file->Read();

		if (entry.size != static_cast<uint64>(fileContent.size()))
		{
			LOG("PackageWriter::Write > Entry size doesn't match read file contents size - " + fullName, LogLevel::Error);
		}

		memcpy(raw + offset, fileContent.data(), entry.size);
		offset += entry.size;
	}
}

