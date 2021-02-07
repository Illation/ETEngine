#pragma once
#include "EditorAsset.h"

#include <EtCore/Content/AssetDatabaseInterface.h>
#include <EtCore/FileSystem/Package/PackageDescriptor.h>


namespace et { namespace core {
	class Directory;
	class AssetDatabase;
}
	REGISTRATION_NS(pl)
}


namespace et {
namespace pl {


//---------------------------------
// EditorAssetDatabase
//
class EditorAssetDatabase final : public core::I_AssetDatabase
{
	// Definitions
	//---------------------
	RTTR_ENABLE()
	REGISTRATION_FRIEND_NS(pl)
public:
	typedef std::vector<EditorAssetBase*> T_AssetList;
private:
	typedef std::vector<T_AssetList> T_CacheList;

	static std::string const s_AssetContentFileExt;

	// static functionality
	//----------------------
	static rttr::type GetCacheType(T_AssetList const& cache);
	static rttr::type GetCacheAssetType(T_AssetList const& cache);

public:
	static void InitDb(EditorAssetDatabase& db, std::string const& path);

	// construct destruct
	//--------------------
	EditorAssetDatabase() = default;
	~EditorAssetDatabase();

private:
	void Init(core::Directory* const directory);

	// accessors
	//-----------
public:
	std::string const& GetAssetPath() const { return m_RootDirectory; }
	core::Directory const* GetDirectory() const { return m_Directory; }
	core::Directory* GetDirectory() { return m_Directory; }

	T_AssetList GetAssetsInPackage(core::HashString const packageId);
	std::vector<core::PackageDescriptor> const& GetPackages() const { return m_Packages; }

	T_AssetList GetAssetsMatchingQuery(std::string const& path,
		bool const recursive,
		std::string const& searchTerm,
		std::vector<rttr::type> const& filteredTypes);

	EditorAssetBase* GetAsset(core::HashString const assetId, bool const reportErrors = true) const;
	EditorAssetBase* GetAsset(core::HashString const assetId, rttr::type const type, bool const reportErrors = true) const; // faster

	bool IsRuntimeAsset(core::I_Asset* const asset) const;

	// Interface
	//-----------
	void IterateAllAssets(core::I_AssetDatabase::T_AssetFunc const& func) override;

	// Functionality
	//---------------------
	void Flush();
	void SetupAllRuntimeAssets();
	void PopulateAssetDatabase(core::AssetDatabase& db) const;
	void RegisterNewAsset(EditorAssetBase* const asset);

	// utility
	//---------
private:
	T_CacheList::iterator FindCacheIt(rttr::type const type);
	T_CacheList::const_iterator FindCacheIt(rttr::type const type) const;

	T_AssetList& FindOrCreateCache(rttr::type const type);

	void RecursivePopulateAssets(core::Directory* const directory);
	void AddAsset(core::File* const configFile);


	// Data
	///////

	core::Directory* m_Directory = nullptr;

	T_CacheList m_AssetCaches;

	// reflected
	std::string m_RootDirectory;
	std::vector<core::PackageDescriptor> m_Packages;
};


} // namespace pl
} // namespace et
