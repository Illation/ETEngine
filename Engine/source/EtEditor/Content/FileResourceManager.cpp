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
// Load the asset database from raw package data, assuming the raw package contains an entry with an id from s_DatabasePath
//
void FileResourceManager::Init()
{
	m_ProjectDir = new Directory(EditorConfig::GetInstance()->GetProjectPath() + s_ResourceDirRelPath + s_DatabasePath, nullptr);
	m_ProjectDir->Mount(true);

	m_EngineDir = new Directory(EditorConfig::GetInstance()->GetEnginePath() + s_ResourceDirRelPath + s_DatabasePath, nullptr);
	m_EngineDir->Mount(true);
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

