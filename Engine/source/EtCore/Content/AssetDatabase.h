#pragma once
#include <rttr/type>

#include "Asset.h"
#include "AssetDatabaseInterface.h"

#include <EtCore/Hashing/Hash.h>
#include <EtCore/FileSystem/Package/PackageDescriptor.h>


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
	I_Asset* GetAsset(HashString const assetId, bool const reportErrors = true) const;
	I_Asset* GetAsset(HashString const assetId, rttr::type const type, bool const reportErrors = true) const; // faster option

	PackageDescriptor const* GetPackage(HashString const packageId) const;

	// Interface
	//-----------
	void IterateAllAssets(I_AssetDatabase::T_AssetFunc const& func) override;

	// Functionality
	//---------------------
	void Flush();

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
