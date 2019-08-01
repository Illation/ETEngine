#pragma once

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

