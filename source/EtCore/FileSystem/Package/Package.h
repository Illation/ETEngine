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

	//---------------------------------
	// Package::PackageFile
	//
	// Data and meta info for a single file within a package
	//
	struct PackageFile
	{
		std::string fileName;

		E_CompressionType compressionType;

		uint64 size;
		uint8 const* content;
	};
	
	Package(uint8 const* const data);
	virtual ~Package() = default;

	PackageFile const* GetFile(T_Hash const id) const;

private:
	std::map<T_Hash, PackageFile> m_Entries;
	uint8 const* m_Data = nullptr;
};