#include "stdafx.h"
#include "EditorAssetDatabase.h"

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Reflection/Serialization.h>


namespace et {
namespace pl {


//=======================
// Editor Asset Database
//=======================


// static
std::string const EditorAssetDatabase::s_AssetContentFileExt = "etac"; // ET Asset Config

//-----------------------------------
// EditorAssetDatabase::GetCacheType
//
rttr::type EditorAssetDatabase::GetCacheType(T_AssetList const& cache)
{
	if (!cache.empty())
	{
		ET_ASSERT(cache[0] != nullptr);
		return cache[0]->GetType();
	}

	return rttr::type::get<std::nullptr_t>();
}


//-----------------------------
// EditorAssetDatabase::d-tor
//
EditorAssetDatabase::~EditorAssetDatabase()
{
	delete m_Directory;

	for (T_AssetList& cache : m_AssetCaches)
	{
		for (EditorAssetBase* asset : cache)
		{
			delete asset;
		}
	}
}

//---------------------------
// EditorAssetDatabase::Init
//
void EditorAssetDatabase::Init(core::Directory* const directory)
{
	m_Directory = directory;
	ET_ASSERT(m_Directory != nullptr);

	RecursivePopulateAssets(m_Directory);
}

//----------------------------------------
// EditorAssetDatabase::FindOrCreateCache
//
EditorAssetDatabase::T_AssetList& EditorAssetDatabase::FindOrCreateCache(rttr::type const type)
{
	auto const foundIt = std::find_if(m_AssetCaches.begin(), m_AssetCaches.end(), [type](T_AssetList const& cache)
		{
			return (EditorAssetDatabase::GetCacheType(cache) == type);
		});

	if (foundIt != m_AssetCaches.cend())
	{
		return *foundIt;
	}

	m_AssetCaches.emplace_back();
	return m_AssetCaches.back();
}

//----------------------------------------------
// EditorAssetDatabase::RecursivePopulateAssets
//
void EditorAssetDatabase::RecursivePopulateAssets(core::Directory* const directory)
{
	std::vector<core::Entry*> const& entries = directory->GetChildren();
	for (core::Entry* const entry : entries)
	{
		switch (entry->GetType())
		{
		case core::Entry::ENTRY_DIRECTORY:
		{
			RecursivePopulateAssets(static_cast<core::Directory*>(entry));
		}
		break;

		case core::Entry::ENTRY_FILE:
		{
			if (entry->GetExtension() == s_AssetContentFileExt)
			{
				AddAsset(static_cast<core::File*>(entry));
			}
		}
		break;
		}
	}
}

//-------------------------------
// EditorAssetDatabase::AddAsset
//
void EditorAssetDatabase::AddAsset(core::File* const configFile)
{
	if (!configFile->Open(core::FILE_ACCESS_MODE::Read))
	{
		ET_ASSERT(false, "couldn't open asset config '%s'", configFile->GetName());
		return;
	}

	std::string const content(core::FileUtil::AsText(configFile->Read()));

	configFile->Close();

	EditorAssetBase* asset = nullptr;

	if (!core::serialization::DeserializeFromJsonString(content, asset))
	{
		ET_ASSERT(false, "failed to deserizlize asset config '%s'", configFile->GetName());
		return;
	}

	ET_ASSERT(asset != nullptr);
	asset->Init(configFile);

	T_AssetList& cache = FindOrCreateCache(asset->GetType());
	cache.push_back(asset);

	LOG(FS("Added asset '%s' to '%s' cache!", asset->GetId().ToStringDbg(), EditorAssetDatabase::GetCacheType(cache).get_name().data()));
}


} // namespace pl
} // namespace et
