#pragma once
#include <rttr/type>

#include "Asset.h"

#include <EtCore/Hashing/Hash.h>


namespace et {
namespace core {


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
		std::type_info const& GetType() const;

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
	T_AssetList GetAssetsMatchingPath(std::string const& path, std::string const& searchTerm);

	I_Asset* GetAsset(HashString const assetId, bool const reportErrors = true) const;
	I_Asset* GetAsset(HashString const assetId, std::type_info const& type, bool const reportErrors = true) const; // faster option

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
