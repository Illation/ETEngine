#include "stdafx.h"
#include "Asset.h"

#include "AssetPointer.h"
#include "ResourceManager.h"

#include <rttr/registration>
#include <EtCore/FileSystem/Package/Package.h>


namespace et {
namespace core {


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<I_Asset>("asset")
		.property("name", &I_Asset::GetName, &I_Asset::SetName)
		.property("path", &I_Asset::GetPath, &I_Asset::SetPath)
		.property("package", &I_Asset::GetPackageName, &I_Asset::SetPackageName)
		.property("references", &I_Asset::GetReferenceNames, &I_Asset::SetReferenceNames)
		;
}


//===================
// Asset Reference
//===================


//---------------------------------
// I_Asset::Reference::c-tor
//
// Init a reference from a name - don't link the asset yet
//
I_Asset::Reference::Reference(std::string const& name) 
	: m_Name(name)
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
	m_Id = GetHash(m_Name);
	m_PackageEntryId = GetHash(m_Path + m_Name);
}

//---------------------------------
// I_Asset::SetPath
//
// Sets the path of an asset and regenerates the package entry ID
//
void I_Asset::SetPath(std::string const& val)
{
	m_Path = val;
	m_PackageEntryId = GetHash(m_Path + m_Name);
}

//---------------------------------
// I_Asset::SetPackageName
//
// Sets the name of the package the asset lives in and generates its package ID
//
void I_Asset::SetPackageName(std::string const& val)
{
	m_PackageName = val;
	m_PackageId = GetHash(m_PackageName);
}

//---------------------------------
// I_Asset::GetReferenceNames
//
// Extract a vector of strings from the reference list
//
std::vector<std::string> I_Asset::GetReferenceNames() const
{
	std::vector<std::string> referenceNames;
	for (Reference const& reference : m_References)
	{
		referenceNames.emplace_back(reference.GetName());
	}
	return referenceNames;
}

//---------------------------------
// I_Asset::SetReferenceNames
//
// Create the reference list from a vector of strings
//
void I_Asset::SetReferenceNames(std::vector<std::string> val)
{
	m_References.clear();
	for (std::string const& refName : val)
	{
		m_References.emplace_back(refName);
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
		LOG("I_Asset::Load > couldn't get data for '" + m_Path + m_Name + std::string("' (") + std::to_string(m_PackageEntryId) 
			+ std::string(") in package '") + m_PackageName + std::string("'"), LogLevel::Warning);
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
