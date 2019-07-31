#pragma once
#include <EtCore/Helper/Singleton.h>

#include "AssetDatabase.h"


// forward decl
class I_Package;


//---------------------------------
// ResourceManager
//
// Class that manages the lifetime of assets
//
class ResourceManager : public Singleton<ResourceManager>
{
public:
	// Definitions
	//---------------------

	friend class Singleton<ResourceManager>;

	static constexpr char s_DatabasePath[] = "asset_database.json";

private:
	// Construct destruct
	//---------------------
	ResourceManager() = default;
	~ResourceManager();
	// Protect from copy construction
	ResourceManager(const ResourceManager& t);
	ResourceManager& operator=(const ResourceManager& t);

public:
	// Init Deinit
	//---------------------
	void InitFromCompiledData();
	void InitFromFile(std::string const& path);

	void Deinit();

	// Managing assets
	//---------------------
	AssetDatabase& GetDatabase() { return m_Database; }

	I_Asset* GetAsset(T_Hash const assetId, std::type_info const& type);

	template <class T>
	Asset<T>* GetAsset(T_Hash const assetId);

	template <class T>
	T const* GetAssetData(T_Hash const assetId);

private:
	// Data
	///////

	AssetDatabase m_Database;
	std::vector<std::pair<T_Hash, I_Package*>> m_Packages;
};


#include "ResourceManager.inl"
