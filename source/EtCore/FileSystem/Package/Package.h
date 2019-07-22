#include <map>

#include <EtCore/Helper/Hash.h>


//---------------------------------
// E_CompressionType
//
// List of supported compression types
//
enum class E_CompressionType : uint8
{
	Store,
	//Brotli, - will be implemented

	COUNT
};


//---------------------------------
// Package
//
// View for a package of data containing multiple possibly compressed files
//
class Package
{
public:
	// definitions
	//--------------

	//---------------------------------
	// Package::PackageFile
	//
	// Data and meta info for a single file within a package
	//
	struct PackageFile
	{
		std::string fileName;
		std::string path;

		E_CompressionType compressionType;

		uint64 size;
		uint8 const* content;
	};

	// ctor dtor
	//--------------
	Package(uint8 const* const data);
	virtual ~Package() = default;

	// utility
	//--------------
	PackageFile const* GetFile(T_Hash const id) const;

private:
	void InitFileListFromData();

	// Data
	///////
	std::map<T_Hash, PackageFile> m_Entries;
	uint8 const* m_Data = nullptr;
};