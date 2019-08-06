#include "stdafx.h"
#include "Asset.h"

#include "AssetPointer.h"
#include "ResourceManager.h"

#include <rttr/registration>
#include <EtCore/FileSystem/Package/Package.h>


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<I_Asset::Reference>("reference")
		.property("name", &I_Asset::Reference::GetName, &I_Asset::Reference::SetName)
		.property("is persistent", &I_Asset::Reference::IsPersistent, &I_Asset::Reference::SetPersistent);

	registration::class_<I_Asset>("asset")
		.property("name", &I_Asset::GetName, &I_Asset::SetName)
		.property("path", &I_Asset::GetPath, &I_Asset::SetPath)
		.property("package", &I_Asset::GetPackageName, &I_Asset::SetPackageName)
		.property("references", &I_Asset::GetReferences, &I_Asset::SetReferences)
		;
}


//===================
// Asset Reference
//===================


//---------------------------------
// I_Asset::Reference::Init
//
// Links the asset pointer
//
void I_Asset::Reference::Init()
{
	m_Asset = ResourceManager::GetInstance()->GetAsset(GetHash(m_Name));
	ET_ASSERT(m_Asset != nullptr, "Couldn't link to asset reference because it wasn't found - '%s'", m_Name.c_str());
}

//---------------------------------
// I_Asset::Reference::Ref
//
// Makes sure the reference is loaded (by creating a smart asset pointer)
//
void I_Asset::Reference::Ref()
{
	ET_ASSERT(m_AssetPtr = nullptr);
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
// I_Asset::Load
//
// Sets the name of an asset and generates its ID
//
void I_Asset::Load()
{
	// Make sure all references are loaded
	for (Reference& ref : m_References)
	{
		ref.Ref();
	}

	// Get the package the asset lives in
	I_Package* const container = ResourceManager::GetInstance()->GetPackage(m_PackageId);
	ET_ASSERT(container != nullptr, "No package (name:'%s', id:'%x') found for asset '%s'", m_PackageName.c_str(), m_PackageId, m_Name.c_str());

	// get binary data from the package
	if (!(container->GetEntryData(m_PackageEntryId, m_LoadData)))
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
	for (Reference& ref : m_References)
	{
		if (!ref.IsPersistent())
		{
			ref.Deref();
		}
	}
}

//---------------------------------
// I_Asset::DereferencePersistent
//
// dereference persistent references 
//
void I_Asset::DereferencePersistent()
{
	for (Reference& ref : m_References)
	{
		if (ref.IsPersistent())
		{
			ref.Deref();
		}
	}
}
