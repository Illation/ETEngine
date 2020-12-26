#include "stdafx.h"
#include "FileResourceManager.h"

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/Reflection/Serialization.h>
#include <EtCore/FileSystem/FileUtil.h>


namespace et {
namespace pl {


//==========================
// File Resource Manager
//==========================


//---------------------------------
// FileResourceManager::c-tor
//
FileResourceManager::FileResourceManager(std::string const& projectPath, std::string const& enginePath)
	: ResourceManager()
	, m_ProjectPath(projectPath)
	, m_EnginePath(enginePath)
{ }

//---------------------------------
// FileResourceManager::Init
//
// Load the asset database from the engine and project files and link them together
//
void FileResourceManager::Init()
{
	// init databases and file directories unlinked 
	EditorAssetDatabase::InitDb(m_ProjectDb, m_ProjectPath + s_ResourceDirRelPath + s_DatabasePath);
	EditorAssetDatabase::InitDb(m_EngineDb, m_EnginePath + s_ResourceDirRelPath + s_DatabasePath);

	// link databases
	auto assetGetter = [this](core::HashString const assetId) -> core::I_Asset*
		{
			EditorAssetBase* ret = m_ProjectDb.GetAsset(assetId, false);

			if (ret == nullptr)
			{
				ret = m_EngineDb.GetAsset(assetId); 
				if (ret == nullptr)
				{
					return nullptr;
				}
			}

			return ret->GetAsset();
		};

	SetAssetReferences(&m_ProjectDb, assetGetter);
	SetAssetReferences(&m_EngineDb, assetGetter);
}

//--------------------------------
// FileResourceManager::LoadAsset
//
void FileResourceManager::LoadAsset(core::I_Asset* const asset)
{
	ET_ASSERT(!(asset->IsLoaded()));

	EditorAssetBase* const editorAsset = GetEditorAsset(asset);
	ET_ASSERT(editorAsset != nullptr);

	editorAsset->Load();
}

//----------------------------------
// FileResourceManager::UnloadAsset
//
void FileResourceManager::UnloadAsset(core::I_Asset* const asset)
{
	ET_ASSERT(asset->IsLoaded());

	EditorAssetBase* const editorAsset = GetEditorAsset(asset);
	ET_ASSERT(editorAsset != nullptr);

	editorAsset->Unload();
}

//--------------------------------------
// FileResourceManager::GetLoadData
//
// Retrieve the data for this asset
//
bool FileResourceManager::GetLoadData(core::I_Asset const* const asset, std::vector<uint8>& outData) const
{
	// figure out which directory we need to search in
	core::Directory const* const searchDir = (IsEngineResource(asset) ? m_EngineDb.GetDirectory() : m_ProjectDb.GetDirectory());

	// find the database file
	core::Entry* const dbEntry = searchDir->GetMountedChild(asset->GetPath() + asset->GetName());

	// make sure we have a valid asset
	if ((dbEntry == nullptr) || (dbEntry->GetType() == core::Entry::ENTRY_DIRECTORY))
	{
		LOG(FS("Asset '%s' not found at location: %s", asset->GetName().c_str(), (searchDir->GetName() + asset->GetPath()).c_str()), 
			core::LogLevel::Warning);
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
core::I_Asset* FileResourceManager::GetAssetInternal(core::HashString const assetId, rttr::type const type, bool const reportErrors)
{
	EditorAssetBase* const ret = GetEditorAsset(assetId, type, reportErrors);
	if (ret == nullptr)
	{
		return nullptr;
	}

	return ret->GetAsset();
}

//-------------------------------------
// FileResourceManager::GetEditorAsset
//
EditorAssetBase* FileResourceManager::GetEditorAsset(core::HashString const assetId, rttr::type const type, bool const reportErrors)
{
	EditorAssetBase* const ret = m_ProjectDb.GetAsset(assetId, type, false);

	if (ret == nullptr)
	{
		return m_EngineDb.GetAsset(assetId, type, reportErrors); // only log if first search fails
	}

	return ret;
}

//-------------------------------------------
// FileResourceManager::GetEditorAsset
//
EditorAssetBase* FileResourceManager::GetEditorAsset(core::I_Asset* const asset, bool const reportErrors)
{
	return GetEditorAsset(asset->GetId(), asset->GetType(), reportErrors);
}

//---------------------------------
// FileResourceManager::IsEngineResource
//
bool FileResourceManager::IsEngineResource(core::I_Asset const* const asset) const
{
	return (m_EngineDb.GetAsset(asset->GetId(), asset->GetType(), false) != nullptr);
}


} // namespace pl
} // namespace et
