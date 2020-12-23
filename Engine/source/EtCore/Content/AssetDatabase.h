#pragma once
#include <rttr/type>

#include "Asset.h"
#include "AssetDatabaseInterface.h"

#include <EtCore/Hashing/Hash.h>


namespace et {
namespace core {


//---------------------------------
// AssetDatabase
//
// Container for all assets and package descriptors
//
struct AssetDatabase final : public I_AssetDatabase
{
public:
	// Definitions
	//---------------------
	typedef std::vector<I_Asset*> T_AssetList;

	static std::vector<rttr::type> GetValidAssetTypes(rttr::type const type, bool const reportErrors);

	class PackageDescriptor final
	{
	public:
		std::string const& GetName() const { return m_Name; }
		void SetName(std::string const& val);

		std::string const& GetPath() const { return m_Path; }
		void SetPath(std::string const& val) { m_Path = val; }

		HashString GetId() const { return m_Id; }
	private:
		// Data
		///////

		// reflected
		std::string m_Name;
		std::string m_Path;

		// derived
		HashString m_Id;

		RTTR_ENABLE()
	};

	struct AssetCache final
	{
		rttr::type GetType() const;
		rttr::type GetAssetType() const;

		T_AssetList cache;

		RTTR_ENABLE()
	};

	// Construct destruct
	//---------------------
	AssetDatabase(bool const ownsAssets = true) : m_OwnsAssets(ownsAssets) {}
	~AssetDatabase();

	// Accessors
	//---------------------
	T_AssetList GetAssetsInPackage(HashString const packageId);
	T_AssetList GetAssetsMatchingQuery(std::string const& path, 
		bool const recursive, 
		std::string const& searchTerm, 
		std::vector<rttr::type> const& filteredTypes);

	I_Asset* GetAsset(HashString const assetId, bool const reportErrors = true) const;
	I_Asset* GetAsset(HashString const assetId, rttr::type const type, bool const reportErrors = true) const; // faster option

	// Interface
	//-----------
	void IterateAllAssets(I_AssetDatabase::T_AssetFunc const& func) override;

	// Functionality
	//---------------------
	void Flush();
	void Merge(AssetDatabase const& other);

	// Data
	////////
	std::vector<PackageDescriptor> packages;
	std::vector<AssetCache> caches;

	RTTR_ENABLE()

private:
	bool m_OwnsAssets = true;
};


} // namespace core
} // namespace et
