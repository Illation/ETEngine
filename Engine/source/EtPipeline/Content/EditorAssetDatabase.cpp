#include "stdafx.h"
#include "EditorAssetDatabase.h"

#include <EtCore/Content/AssetDatabase.h>
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

//----------------------------------------
// EditorAssetDatabase::GetCacheAssetType
//
rttr::type EditorAssetDatabase::GetCacheAssetType(T_AssetList const& cache)
{
	if (!cache.empty())
	{
		ET_ASSERT(cache[0] != nullptr);
		return rttr::type::get(*(cache[0]));
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
	ET_ASSERT(m_Directory->IsMounted());

	RecursivePopulateAssets(m_Directory);
}

//---------------------------------------------
// EditorAssetDatabase::GetAssetsMatchingQuery
//
EditorAssetDatabase::T_AssetList EditorAssetDatabase::GetAssetsMatchingQuery(std::string const& path, 
	bool const recursive, 
	std::string const& searchTerm, 
	std::vector<rttr::type> const& filteredTypes)
{
	T_AssetList outAssets;

	std::string lowerSearch = searchTerm;
	std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

	// caches for every asset type 
	for (T_AssetList& cache : m_AssetCaches)
	{
		if (filteredTypes.size() > 0u)
		{
			rttr::type const cacheType = GetCacheAssetType(cache);
			if (std::find(filteredTypes.begin(), filteredTypes.end(), cacheType) == filteredTypes.cend())
			{
				continue;
			}
		}

		// every asset per cache
		for (EditorAssetBase* const editorAsset : cache)
		{
			core::I_Asset const* const asset = editorAsset->GetAsset();
			if ((asset->GetPath().rfind(path, 0) == 0) && (recursive || (asset->GetPath().length() == path.length())))
			{
				bool matchesSearch = true;
				if (lowerSearch.length() != 0u)
				{
					std::string lowerAsset = asset->GetPath() + asset->GetName();
					std::transform(lowerAsset.begin(), lowerAsset.end(), lowerAsset.begin(), ::tolower);
					matchesSearch = (lowerAsset.find(lowerSearch) != std::string::npos);
				}

				if (matchesSearch)
				{
					outAssets.emplace_back(editorAsset);
				}
			}
		}
	}

	return outAssets;
}

//-------------------------------
// EditorAssetDatabase::GetAsset
//
EditorAssetBase* EditorAssetDatabase::GetAsset(core::HashString const assetId, bool const reportErrors) const
{
	// in this version we loop over all caches
	for (T_AssetList const& cache : m_AssetCaches)
	{
		// try finding our asset by its ID in the cache
		auto foundAssetIt = std::find_if(cache.begin(), cache.end(), [assetId](EditorAssetBase* asset)
			{
				return asset->GetId() == assetId;
			});

		if (foundAssetIt != cache.cend())
		{
			return *foundAssetIt;
		}
	}

	// didn't find an asset in any cache, return null
	if (reportErrors)
	{
		ET_ASSERT(false, "Couldn't find asset with ID '%s'!", assetId.ToStringDbg());
	}

	return nullptr;
}

//-------------------------------
// EditorAssetDatabase::GetAsset
//
EditorAssetBase* EditorAssetDatabase::GetAsset(core::HashString const assetId, rttr::type const type, bool const reportErrors) const
{
	std::vector<rttr::type> const assetTypes = core::AssetDatabase::GetValidAssetTypes(type, reportErrors);

	for (rttr::type const assetType : assetTypes)
	{
		// Try finding a cache containing our type
		auto const foundCacheIt = FindCacheIt(type);
		if (foundCacheIt != m_AssetCaches.cend())
		{
			// try finding our asset by its ID in the cache
			auto const foundAssetIt = std::find_if(foundCacheIt->cbegin(), foundCacheIt->cend(), [assetId](EditorAssetBase const* const asset)
				{
					return asset->GetId() == assetId;
				});

			if (foundAssetIt != foundCacheIt->cend())
			{
				return *foundAssetIt;
			}
		}
	}

	if (reportErrors)
	{
		ET_ASSERT(false, "Couldn't find asset with ID '%s'!", assetId.ToStringDbg());
	}

	return nullptr;
}

//---------------------------------------
// EditorAssetDatabase::IterateAllAssets
//
void EditorAssetDatabase::IterateAllAssets(core::I_AssetDatabase::T_AssetFunc const& func)
{
	for (T_AssetList& cache : m_AssetCaches)
	{
		for (EditorAssetBase* const asset : cache)
		{
			func(asset->GetAsset());
		}
	}
}

//-----------------------------
// EditorAssetDatabase::Flush
//
void EditorAssetDatabase::Flush()
{
	for (T_AssetList& cache : m_AssetCaches)
	{
		for (EditorAssetBase* asset : cache)
		{
			if (asset->GetAsset()->GetRefCount() <= 0u && asset->GetAsset()->IsLoaded())
			{
				asset->Unload(true);
			}
		}
	}
}

//----------------------------------
// EditorAssetDatabase::FindCacheIt
//
EditorAssetDatabase::T_CacheList::iterator EditorAssetDatabase::FindCacheIt(rttr::type const type)
{
	return std::find_if(m_AssetCaches.begin(), m_AssetCaches.end(), [type](T_AssetList const& cache)
		{
			return (EditorAssetDatabase::GetCacheType(cache) == type);
		});
}

//----------------------------------
// EditorAssetDatabase::FindCacheIt
//
EditorAssetDatabase::T_CacheList::const_iterator EditorAssetDatabase::FindCacheIt(rttr::type const type) const
{
	return std::find_if(m_AssetCaches.cbegin(), m_AssetCaches.cend(), [type](T_AssetList const& cache)
		{
			return (EditorAssetDatabase::GetCacheType(cache) == type);
		});
}

//----------------------------------------
// EditorAssetDatabase::FindOrCreateCache
//
EditorAssetDatabase::T_AssetList& EditorAssetDatabase::FindOrCreateCache(rttr::type const type)
{
	auto const foundIt = FindCacheIt(type);

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
