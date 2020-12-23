#pragma once

#include "EditorAsset.h"


namespace et { namespace core {
	class Directory;
} }


namespace et {
namespace pl {


//---------------------------------
// EditorAssetDatabase
//
class EditorAssetDatabase final
{
	// Definitions
	//---------------------
	typedef std::vector<EditorAssetBase*> T_AssetList;
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

	// utility
	//---------
private:
	T_AssetList& FindOrCreateCache(rttr::type const type);
	void RecursivePopulateAssets(core::Directory* const directory);
	void AddAsset(core::File* const configFile);


	// Data
	///////

	core::Directory* m_Directory = nullptr;
	std::vector<T_AssetList> m_AssetCaches;
};


} // namespace pl
} // namespace et
