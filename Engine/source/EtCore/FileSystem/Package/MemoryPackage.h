#pragma once

#include "Package.h"


namespace et {
namespace core {


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
	PackageEntry const* GetEntry(HashString const id) const;
	bool GetEntryData(HashString const id, std::vector<uint8>& outData) override;

private:
	void InitFileListFromData();

	// Data
	///////
	std::unordered_map<HashString, PackageEntry> m_Entries;
	uint8 const* m_Data = nullptr;
};


} // namespace core
} // namespace et
