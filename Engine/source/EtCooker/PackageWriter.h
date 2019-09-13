#pragma once
#include <EtCore/FileSystem/Package/PackageDataStructure.h>

class File;


//---------------------------------
// PackageWriter
//
// Writes a list of files to a binary package/archive 
//
class PackageWriter final
{
public:
	// type definitions
	//------------------
	struct FileEntryInfo
	{
		FileEntryInfo(PkgEntry const& lEntry, File* const lFile, std::string const& lRelName);

		PkgEntry entry;
		File* file;
		std::string relName;
	};

	// c-tor d-tor
	//------------------
	PackageWriter() = default;
	~PackageWriter();

	// functionality
	//------------------
	void AddFile(File* const file, std::string const& rootDir, E_CompressionType const compression);

	void Write(std::vector<uint8>& data);

	// Data
	///////
private:
	std::vector<FileEntryInfo> m_Files;
};

