// Data structures for the layout of a package

#pragma once
#include <EtCore/Hashing/Hash.h>


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
	T_Hash fileId;
	uint64 offset;
};

//---------------------------------
// PkgEntry
//
// Meta info for a package file entry
//
struct PkgEntry
{
	T_Hash fileId;
	E_CompressionType compressionType;
	uint16 nameLength;
	uint64 size;
};
