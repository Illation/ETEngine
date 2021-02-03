#include "stdafx.h"
#include "Asset.h"

#include "AssetPointer.h"
#include "ResourceManager.h"

#include <EtCore/FileSystem/Package/Package.h>


namespace et {
namespace core {


// reflection
RTTR_REGISTRATION
{
	rttr::registration::class_<I_Asset>("asset")
		.property("name", &I_Asset::GetName, &I_Asset::SetName)
		.property("path", &I_Asset::GetPath, &I_Asset::SetPath)
		.property("package", &I_Asset::m_PackageId)
		.property("references", &I_Asset::GetReferenceIds, &I_Asset::SetReferenceIds);
}


//===================
// Asset Reference
//===================


//---------------------------------
// I_Asset::Reference::c-tor
//
// Init a reference from a name - don't link the asset yet
//
I_Asset::Reference::Reference(HashString const id)
	: m_Id(id)
{ }

//---------------------------------
// I_Asset::Reference::d-tor
//
// Make sure to remove the asset pointer so the assets ref count goes down
//
I_Asset::Reference::~Reference()
{
	SafeDelete(m_AssetPtr);
}

//---------------------------------
// I_Asset::Reference::Ref
//
// Makes sure the reference is loaded (by creating a smart asset pointer)
//
void I_Asset::Reference::Ref()
{
	ET_ASSERT(m_AssetPtr == nullptr);
	m_AssetPtr = new I_AssetPtr(m_Asset);
}

//---------------------------------
// I_Asset::Reference::Deref
//
// Allows the reference to be unloaded if needed (by deleting the smart asset pointer)
//
void I_Asset::Reference::Deref()
{
	ET_ASSERT(m_AssetPtr != nullptr);
	delete m_AssetPtr;
	m_AssetPtr = nullptr;
}


//===================
// Asset
//===================


// Construct destruct
///////////////

//---------------------------------
// I_Asset::~I_Asset
//
// I_Asset destructor
//
I_Asset::~I_Asset()
{
	Unload();
}


// Utility
///////////////

//---------------------------------
// I_Asset::SetName
//
// Sets the name of an asset and generates its ID and package entry ID
//
void I_Asset::SetName(std::string const& val)
{
	m_Name = val;
	//m_Id = m_Name.c_str();
	m_Id = (m_Path + m_Name).c_str();
	m_PackageEntryId = (m_Path + m_Name).c_str();
}

//---------------------------------
// I_Asset::SetPath
//
// Sets the path of an asset and regenerates the package entry ID
//
void I_Asset::SetPath(std::string const& val)
{
	m_Path = val;
	m_Id = (m_Path + m_Name).c_str();
	m_PackageEntryId = (m_Path + m_Name).c_str();
}

//---------------------------------
// I_Asset::GetReferenceIds
//
// Extract a vector of strings from the reference list
//
std::vector<HashString> I_Asset::GetReferenceIds() const
{
	std::vector<HashString> referenceIds;
	for (Reference const& reference : m_References)
	{
		referenceIds.emplace_back(reference.GetId());
	}

	return referenceIds;
}

//---------------------------------
// I_Asset::SetReferenceIds
//
// Create the reference list from a vector of strings
//  - can't make list a const ref thanks to rttr
//
void I_Asset::SetReferenceIds(std::vector<HashString> val)
{
	m_References.clear();
	for (HashString const refId : val)
	{
		m_References.emplace_back(refId);
	}
}

//---------------------------------
// I_Asset::Load
//
// Sets the name of an asset and generates its ID
//
void I_Asset::Load()
{
	// Make sure all references are loaded
	for (Reference& reference : m_References)
	{
		reference.Ref();
	}

	// get binary data from the package
	if (!(ResourceManager::Instance()->GetLoadData(this, m_LoadData)))
	{
		ET_ASSERT(false, "Couldn't get data for '%s' (%i) in package '%s'", 
			m_PackageEntryId.ToStringDbg(), 
			m_PackageEntryId.Get(), 
			m_PackageId.ToStringDbg());
		return;
	}

	// let the asset load from binary data
	if (!LoadFromMemory(m_LoadData))
	{
		LOG("I_Asset::Load > Failed loading asset from memory, name: '" + m_Name + std::string("'"), LogLevel::Warning);
	}

	if (!m_IsPersistent)
	{
		m_LoadData.clear();
	}

	// dereference non persistent references 
	for (Reference& reference : m_References)
	{
		reference.Deref();
	}
}

//---------------------------------
// I_Asset::Unload
//
// Either unload an asset now or wait for the asset manager to flush
//  - waiting allows us to keep common non persistent assets around during batched load processes until everything is done
//
void I_Asset::Unload(bool const force)
{
	ResourceManager const* const resMan = ResourceManager::Instance();

	if (force || resMan == nullptr || !(resMan->IsUnloadDeferred()))
	{
		UnloadInternal();
	}
}


} // namespace core
} // namespace et
