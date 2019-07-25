#pragma once
#include <EtCore/Helper/Singleton.h>

#include <EtCore/Helper/Hash.h>

#include "Asset.h"

#include <rttr/type>


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

	//---------------------------------
	// AssetDatabase
	//
	// Container for all assets
	//
	struct AssetDatabase final
	{
	public:
		// Definitions
		//---------------------
		typedef std::vector<I_Asset*> T_AssetList;
		struct AssetCache final
		{
			std::type_info const& GetType() const;

			T_AssetList cache;

			RTTR_ENABLE()
		};

		// Data
		////////
		std::vector<AssetCache> caches;

		RTTR_ENABLE()
	};

private:
	// Construct destruct
	//---------------------
	ResourceManager() = default;
	~ResourceManager();
	// Protect from copy construction
	ResourceManager(const ResourceManager& t);
	ResourceManager& operator=(const ResourceManager& t);

public:
	// Managing assets
	//---------------------
	void Init();
	void InitFromFile(std::string const& path);

	void Deinit();

	I_Asset* GetAsset(T_Hash const assetId, std::type_info const& type);

	template <class T>
	Asset<T>* GetAsset(T_Hash const assetId);

	template <class T>
	T const* GetAssetData(T_Hash const assetId);

private:
	// Data
	///////

	AssetDatabase m_Database;
};

#include "ResourceManager.inl"
