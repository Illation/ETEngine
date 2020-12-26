#include "stdafx.h"
#include "EditorAssetDatabase.h"

#include <rttr/registration>

#include <EtCore/Content/AssetDatabase.h>
#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Reflection/Serialization.h>


namespace et {
namespace pl {


//=======================
// Editor Asset Database
//=======================


// reflection
RTTR_REGISTRATION
{
	rttr::registration::class_<EditorAssetDatabase>("editor asset database")
		.property("packages", &EditorAssetDatabase::m_Packages);
}


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
// EditorAssetDatabase::InitDb
//
// Initialize a database given a filepath, including all necessary deserialization
//
void EditorAssetDatabase::InitDb(EditorAssetDatabase& db, std::string const& path)
{
	std::string dirPath = core::FileUtil::ExtractPath(path);
	std::string fileName = core::FileUtil::ExtractName(path);

	// mount the directory
	core::Directory* const dir = new core::Directory(dirPath, nullptr, true);
	dir->Mount(true);

	// find the database file
	core::Entry* const dbEntry = dir->GetMountedChild(fileName);
	if (dbEntry != nullptr) // if this is a new project we may not have a database yet
	{
		ET_ASSERT(dbEntry->GetType() == core::Entry::ENTRY_FILE);

		core::File* const dbFile = static_cast<core::File*>(dbEntry);
		dbFile->Open(core::FILE_ACCESS_MODE::Read);

		// deserialize the database from that files content
		if (!(core::serialization::DeserializeFromJsonString(core::FileUtil::AsText(dbFile->Read()), db)))
		{
			LOG(FS("FileResourceManager::Init > unable to deserialize asset DB at '%s'", dbEntry->GetName().c_str()), core::LogLevel::Error);
		}
	}
	else
	{
		LOG(FS("No Database file found at '%s'", (dir->GetName() + fileName).c_str()));
	}

	// --
	db.Init(dir);
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

//-----------------------------------------
// EditorAssetDatabase::GetAssetsInPackage
//
EditorAssetDatabase::T_AssetList EditorAssetDatabase::GetAssetsInPackage(core::HashString const packageId)
{
	T_AssetList outAssets;

	// caches for every asset type 
	for (T_AssetList& cache : m_AssetCaches)
	{
		// every asset per cache
		for (EditorAssetBase* const asset : cache)
		{
			if (asset->GetAsset()->GetPackageId() == packageId)
			{
				outAssets.emplace_back(asset);
			}
		}
	}

	return outAssets;
}

//---------------------------------------------
// EditorAssetDatabase::GetAssetsMatchingQuery
//
// finds all assets that are contained in a path
//  - if recursive is enabled assets are also found in subdirectories
//  - if searchTerm isn't an empty string, only assets containing the search term will be returned
//  - if filteredTypes isn't empty, only assets of types contained in filtered types are returned
//
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
		auto const foundCacheIt = FindCacheIt(assetType);
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

//--------------------------------------------
// EditorAssetDatabase::PopulateAssetDatabase
//
void EditorAssetDatabase::PopulateAssetDatabase(core::AssetDatabase& db) const 
{
	// add packages
	for (core::PackageDescriptor const& desc : m_Packages)
	{
		auto packageIt = std::find_if(db.packages.cbegin(), db.packages.cend(), [&desc](core::PackageDescriptor const& lhs)
			{
				return lhs.GetId() == desc.GetId();
			});

		// if the other DB contains a package that this DB doesn't know of, add it
		if (packageIt == db.packages.cend())
		{
			db.packages.emplace_back(desc);
		}
		else
		{
			// if both have the same package, ensure they agree on its details
			ET_ASSERT(packageIt->GetPath() == desc.GetPath(),
				"DBs disagree on paths for package '%s'! this: '%s' - other: '%s'",
				desc.GetName().c_str(),
				packageIt->GetPath().c_str(),
				desc.GetPath().c_str());
		}
	}

	// add caches
	for (T_AssetList const& rhCache : m_AssetCaches)
	{
		auto cacheIt = std::find_if(db.caches.begin(), db.caches.end(), [&rhCache](core::AssetDatabase::AssetCache const& lhCache)
			{
				return (lhCache.GetType() == GetCacheType(rhCache));
			});

		if (cacheIt == db.caches.cend())
		{
			db.caches.emplace_back();
			cacheIt = std::prev(db.caches.end());
		}

		core::AssetDatabase::AssetCache& lhCache = *cacheIt;

		// insert assets
		for (EditorAssetBase* const editorAsset : rhCache)
		{
			core::I_Asset* const rhAsset = editorAsset->GetAsset();

			// Ensure the asset doesn't already exist
			auto const assetIt = std::find_if(lhCache.cache.cbegin(), lhCache.cache.cend(), [rhAsset](core::I_Asset const* const lhAsset)
				{
					return (lhAsset->GetId() == rhAsset->GetId());
				});

			// if the to merge asset is unique add it
			if (assetIt == lhCache.cache.cend())
			{
				// check we have a package descriptor for the new asset
				ET_ASSERT(std::find_if(db.packages.cbegin(), db.packages.cend(), [rhAsset](core::PackageDescriptor const& lhPackage)
				{
					return lhPackage.GetId() == rhAsset->GetPackageId();
				}) != db.packages.cend() || rhAsset->GetPackageId() == 0u,
					"Asset merged into DB, but DB doesn't contain package '%s'",
					rhAsset->GetPackageId().ToStringDbg());

				lhCache.cache.emplace_back(rhAsset);
			}
			else
			{
				// if the asset is already included, that's an issue
				LOG(FS("AssetDatabase::Merge > Asset already contained in this DB! "
					"Name: '%s', Path: '%s', Merge Path: '%s', Package: '%s', Merge Package: '%s'",
					rhAsset->GetName().c_str(),
					(*assetIt)->GetPath().c_str(),
					rhAsset->GetPath().c_str(),
					(*assetIt)->GetPackageId().ToStringDbg(),
					rhAsset->GetPackageId().ToStringDbg()), 
					core::LogLevel::Error);
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
