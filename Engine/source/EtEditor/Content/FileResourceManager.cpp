#include "stdafx.h"
#include "FileResourceManager.h"

#include <EtCore/Reflection/Serialization.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>

#include <EtEditor/Util/EditorConfig.h>


namespace et {
namespace edit {


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
	auto assetGetter = [this](core::HashString const assetId)
	{
		core::I_Asset* ret = m_ProjectDb.GetAsset(assetId, false);

		if (ret == nullptr)
		{
			return m_EngineDb.GetAsset(assetId); // we only need to log if the first search fails
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
bool FileResourceManager::GetLoadData(core::I_Asset const* const asset, std::vector<uint8>& outData) const
{
	// figure out which directory we need to search in
	core::Directory const* const searchDir = (IsEngineResource(asset) ? m_EngineDir : m_ProjectDir);

	// find the database file
	core::Entry* const dbEntry = searchDir->GetMountedChild(asset->GetPath() + asset->GetName());

	// make sure we have a valid asset
	if ((dbEntry == nullptr) || (dbEntry->GetType() == core::Entry::ENTRY_DIRECTORY))
	{
		LOG(FS("Asset '%s' not found at location: %s", asset->GetName().c_str(), (searchDir->GetName() + asset->GetPath()).c_str()), core::LogLevel::Warning);
		return false;
	}

	// open the file
	core::File* const dbFile = static_cast<core::File*>(dbEntry);
	if (!(dbFile->Open(core::FILE_ACCESS_MODE::Read)))
	{
		LOG(FS("Failed to open file '%s'", dbFile->GetName()), core::LogLevel::Warning);
		return false;
	}

	// read the file into our load data array
	outData = dbFile->Read();
	return true;
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
core::I_Asset* FileResourceManager::GetAssetInternal(core::HashString const assetId, std::type_info const& type, bool const reportErrors)
{
	core::I_Asset* ret = m_ProjectDb.GetAsset(assetId, type, false);

	if (ret == nullptr)
	{
		return m_EngineDb.GetAsset(assetId, type, reportErrors); // only log if first search fails
	}

	return ret;
}

//---------------------------------
// FileResourceManager::InitDb
//
// Initialize an individual Asset Database
//
void FileResourceManager::InitDb(core::AssetDatabase& db, core::Directory*& dir, std::string const& path)
{
	// mount the directory
	dir = new core::Directory(path + s_ResourceDirRelPath, nullptr, true);
	dir->Mount(true);

	// find the database file
	core::Entry* const dbEntry = dir->GetMountedChild(s_DatabasePath);
	ET_ASSERT(dbEntry != nullptr);
	ET_ASSERT(dbEntry->GetType() == core::Entry::ENTRY_FILE);

	core::File* const dbFile = static_cast<core::File*>(dbEntry);
	dbFile->Open(core::FILE_ACCESS_MODE::Read);

	// deserialize the database from that files content
	if (!(core::serialization::DeserializeFromJsonString(core::FileUtil::AsText(dbFile->Read()), db)))
	{
		LOG(FS("FileResourceManager::Init > unable to deserialize asset DB at '%s'", dbEntry->GetName().c_str()), core::LogLevel::Error);
	}
}

//---------------------------------
// FileResourceManager::IsEngineResource
//
bool FileResourceManager::IsEngineResource(core::I_Asset const* const asset) const
{
	return (m_EngineDb.GetAsset(asset->GetId(), asset->GetType(), false) != nullptr);
}


} // namespace edit
} // namespace et
