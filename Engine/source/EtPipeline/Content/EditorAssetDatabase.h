#pragma once
#include "EditorAsset.h"

#include <EtCore/Content/AssetDatabaseInterface.h>


namespace et { namespace core {
	class Directory;
} }


namespace et {
namespace pl {


//---------------------------------
// EditorAssetDatabase
//
class EditorAssetDatabase final : public core::I_AssetDatabase
{
	// Definitions
	//---------------------
	typedef std::vector<EditorAssetBase*> T_AssetList;
	typedef std::vector<T_AssetList> T_CacheList;

	static std::string const s_AssetContentFileExt;

	// static functionality
	//----------------------
	static rttr::type GetCacheType(T_AssetList const& cache);

	// construct destruct
	//--------------------
public:
	EditorAssetDatabase() = default;
	~EditorAssetDatabase();

	void Init(core::Directory* const directory);

	// accessors
	//-----------
	core::Directory const* GetDirectory() const { return m_Directory; }

	EditorAssetBase* GetAsset(core::HashString const assetId, bool const reportErrors = true) const;
	EditorAssetBase* GetAsset(core::HashString const assetId, rttr::type const type, bool const reportErrors = true) const; // faster

	// Interface
	//-----------
	void IterateAllAssets(core::I_AssetDatabase::T_AssetFunc const& func) override;

	// Functionality
	//---------------------
	void Flush();

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
};


} // namespace pl
} // namespace et
