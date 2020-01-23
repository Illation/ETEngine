#pragma once
#include <EtCore/FileSystem/Package/PackageDataStructure.h>


namespace et { namespace core {
	class File;
} }


namespace et {
namespace cooker {


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
		FileEntryInfo(core::PkgEntry const& lEntry, core::File* const lFile, std::string const& lRelName);

		core::PkgEntry entry;
		core::File* file;
		std::string relName;
	};

	// c-tor d-tor
	//------------------
	PackageWriter() = default;
	~PackageWriter();

	// functionality
	//------------------
	void AddFile(core::File* const file, std::string const& rootDir, core::E_CompressionType const compression);
	void RemoveFile(core::File* const file);
	void Cleanup();

	void Write(std::vector<uint8>& data);

	// Data
	///////
private:
	std::vector<FileEntryInfo> m_Files;
};


} // namespace cooker
} // namespace et
