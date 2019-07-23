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

	// finally we can write our data to the vector
	//---------------------------
}

