#pragma once

#include <map>

#include "PackageDataStructure.h"


//---------------------------------
// I_Package
//
// Interface for packages that allows accessing data by its ID
// Packages can live in a file or in memory, and can contain multiple files which may be compressed
//
class I_Package
{
public:
	virtual ~I_Package() = default;

	virtual uint8 const* GetEntryData(T_Hash const id, uint64& size) = 0;
};

//---------------------------------
// MemoryPackage
//
// Package that acts as a view into preloaded memory
//
class MemoryPackage final : public I_Package
{
public:
	// definitions
	//--------------

	//---------------------------------
	// MemoryPackage::PackageEntry
	//
	// Data and meta info for a single entry within a package
	//
	struct PackageEntry
	{
		std::string fileName;
		std::string path;

		E_CompressionType compressionType;

		uint64 size;
		uint8 const* content;
	};

	// ctor dtor
	//--------------
	MemoryPackage(uint8 const* const data);
	virtual ~MemoryPackage() = default;

	// utility
	//--------------
	PackageEntry const* GetEntry(T_Hash const id) const;
	uint8 const* GetEntryData(T_Hash const id, uint64& size) override;

private:
	void InitFileListFromData();

	// Data
	///////
	std::map<T_Hash, PackageEntry> m_Entries;
	uint8 const* m_Data = nullptr;
};