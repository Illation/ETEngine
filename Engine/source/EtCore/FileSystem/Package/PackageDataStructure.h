// Data structures for the layout of a package

#pragma once
#include <EtCore/Hashing/Hash.h>


namespace et {
namespace core {


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
// PkgHeader
//
// Minimal file data for the central directory
//
struct PkgHeader
{
	uint64 numEntries;
};

//---------------------------------
// PkgFileInfo
//
// Minimal file data for the central directory
//
struct PkgFileInfo
{
	HashString fileId;
	uint64 offset;
};

//---------------------------------
// PkgEntry
//
// Meta info for a package file entry
//
struct PkgEntry
{
	HashString fileId;
	E_CompressionType compressionType;
	uint16 nameLength;
	uint64 size;
};


} // namespace core
} // namespace et
