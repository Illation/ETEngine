#pragma once

#include "PackageDataStructure.h"


namespace et {
namespace core {


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

	// Read the package entry data into 'outData'
	// If no entry was found for the ID, we return false and out data is undefined.
	virtual bool GetEntryData(T_Hash const id, std::vector<uint8>& outData) = 0;
};


} // namespace core
} // namespace et
