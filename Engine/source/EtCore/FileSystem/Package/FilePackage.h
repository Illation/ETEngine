#pragma once

#include <map>

#include "Package.h"


namespace et {
namespace core {


// forward decl
class File;


//---------------------------------
// FilePackage
//
// Package that lives in a file and is loaded in individual chunks
//
class FilePackage final : public I_Package
{
public:
	// definitions
	//--------------

	static std::string const s_PackageFileExtension; 

	//---------------------------------
	// FilePackage::PackageEntry
	//
	// Data and meta info for a single entry within a package
	//
	struct PackageEntry
	{
		std::string fileName;
		std::string path;

		E_CompressionType compressionType;

		uint64 size;
		uint64 offset;
	};

	// ctor dtor
	//--------------
	FilePackage(std::string const& path);
	virtual ~FilePackage();

	// utility
	//--------------
	PackageEntry const* GetEntry(T_Hash const id) const;
	bool GetEntryData(T_Hash const id, std::vector<uint8>& outData) override;

private:
	void LoadFileList();

	// Data
	///////
	std::map<T_Hash, PackageEntry> m_Entries;
	File* m_File = nullptr;
};


} // namespace core
} // namespace et
