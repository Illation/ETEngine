#include "stdafx.h"
#include "FileResourceManager.h"

#include <EtCore/Reflection/Serialization.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>

#include <EtEditor/EditorConfig.h>


//==========================
// File Resource Manager
//==========================


//---------------------------------
// FileResourceManager::c-tor
//
FileResourceManager::FileResourceManager()
	: ResourceManager()
{ }

//---------------------------------
// FileResourceManager::Init
//
// Load the asset database from the engine and project files and link them together
//
void FileResourceManager::Init()
{
	// init databases and file directories unlinked 
	InitDb(m_ProjectDb, m_ProjectDir, EditorConfig::GetInstance()->GetProjectPath());
	InitDb(m_EngineDb, m_EngineDir, EditorConfig::GetInstance()->GetEnginePath());

	// link databases
	auto assetGetter = [this](T_Hash const assetId)
	{
		I_Asset* const ret = m_ProjectDb.GetAsset(assetId);

		if (ret == nullptr)
		{
			return m_EngineDb.GetAsset(assetId);
		}

		return ret;
	};

	SetAssetReferences(m_ProjectDb, assetGetter);
	SetAssetReferences(m_EngineDb, assetGetter);
}

//---------------------------------
// FileResourceManager::Deinit
//
// Cleans up packages
//
void FileResourceManager::Deinit()
{
	SafeDelete(m_ProjectDir);
	SafeDelete(m_EngineDir);
}

//--------------------------------------
// FileResourceManager::GetLoadData
//
// Retrieve the data for this asset
//
bool FileResourceManager::GetLoadData(I_Asset const* const asset, std::vector<uint8>& outData) const
{
	return false;
}

//---------------------------------
// FileResourceManager::Flush
//
// Flush both databases
//
void FileResourceManager::Flush()
{
	m_ProjectDb.Flush();
	m_EngineDb.Flush();
}

//-------------------------------------------
// FileResourceManager::GetAssetInternal
//
// Tries finding the asset in either asset database
//
I_Asset* FileResourceManager::GetAssetInternal(T_Hash const assetId, std::type_info const& type)
{
	I_Asset* const ret = m_ProjectDb.GetAsset(assetId, type);

	if (ret == nullptr)
	{
		return m_EngineDb.GetAsset(assetId, type);
	}

	return ret;
}

//---------------------------------
// FileResourceManager::InitDb
//
// Initialize an individual Asset Database
//
void FileResourceManager::InitDb(AssetDatabase& db, Directory*& dir, std::string const& path)
{
	// mount the directory
	dir = new Directory(path + s_ResourceDirRelPath, nullptr, true);
	dir->Mount(true);

	// find the database file
	Entry* const dbEntry = dir->GetMountedChild(s_DatabasePath);
	ET_ASSERT(dbEntry != nullptr);
	ET_ASSERT(dbEntry->GetType() == Entry::ENTRY_FILE);

	File* const dbFile = static_cast<File*>(dbEntry);
	dbFile->Open(FILE_ACCESS_MODE::Read);

	// deserialize the database from that files content
	if (!(serialization::DeserializeFromJsonString(FileUtil::AsText(dbFile->Read()), db)))
	{
		LOG(FS("FileResourceManager::Init > unable to deserialize asset DB at '%s'", dbEntry->GetName().c_str()), LogLevel::Error);
	}
}
