#pragma once

#include <rttr/type>

#include "Asset.h"

#include <EtCore/Helper/Hash.h>


//---------------------------------
// AssetDatabase
//
// Container for all assets and package descriptors
//
struct AssetDatabase final
{
public:
	// Definitions
	//---------------------
	typedef std::vector<I_Asset*> T_AssetList;

	class PackageDescriptor final
	{
	public:
		std::string const& GetName() const { return m_Name; }
		void SetName(std::string const& val);

		std::string const& GetPath() const { return m_Path; }
		void SetPath(std::string const& val) { m_Path = val; }

		T_Hash GetId() const { return m_Id; }
	private:
		// Data
		///////

		// reflected
		std::string m_Name;
		std::string m_Path;

		// derived
		T_Hash m_Id;

		RTTR_ENABLE()
	};

	struct AssetCache final
	{
		std::type_info const& GetType() const;

		T_AssetList cache;

		RTTR_ENABLE()
	};

	~AssetDatabase();

	T_AssetList GetAssetsInPackage(T_Hash const packageId);

	I_Asset* GetAsset(T_Hash const assetId);
	I_Asset* GetAsset(T_Hash const assetId, std::type_info const& type);

	void Merge(AssetDatabase const& other);

	// Data
	////////
	std::vector<PackageDescriptor> packages;
	std::vector<AssetCache> caches;

	RTTR_ENABLE()
};

