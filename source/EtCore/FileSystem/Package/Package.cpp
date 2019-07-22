#include "stdafx.h"

#include "Package.h"


//=====================
// Viewport
//=====================


//---------------------------------
// Package::Package
//
// Construct a package with a pointer to its data, initialize the entry map
//
Package::Package(uint8 const* const data)
	: m_Data(data)
{

}

//---------------------------------
// Package::GetFile
//
// Get a package file from a package using its hashed ID
//
Package::PackageFile const* Package::GetFile(T_Hash const id) const
{
	return nullptr;
}
