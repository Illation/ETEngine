#include "stdafx.h"
#include "PackageDescriptor.h"


namespace et {
namespace core {
	

//====================
// Package Descriptor
//====================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<PackageDescriptor>("package")
		.property("name", &PackageDescriptor::GetName, &PackageDescriptor::SetName)
		.property("path", &PackageDescriptor::m_Path)
		.property("is runtime", &PackageDescriptor::m_IsRuntime);
}


//---------------------------------
// PackageDescriptor::SetName
//
// Sets the name of a package generator and generates its ID
//
void PackageDescriptor::SetName(std::string const& val)
{
	m_Name = val;
	m_Id = m_Name.c_str();
}

//---------------------------------
// PackageDescriptor::GetId
//
// In source file to help with linking
//
HashString PackageDescriptor::GetId() const
{
	return m_Id;
}


} // namespace core
} // namespace et
