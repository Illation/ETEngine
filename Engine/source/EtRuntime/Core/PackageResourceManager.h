#pragma once
#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Content/AssetDatabase.h>


// forward decl
class I_Package;


//---------------------------------
// PackageResourceManager
//
// Resource manager implementation that uses packages
//
class PackageResourceManager : public ResourceManager 
{
	// Definitions
	//---------------------
public:
	friend class ResourceManager;
	typedef std::pair<T_Hash, I_Package*> T_IndexedPackage;

	// Construct destruct
	//---------------------
	PackageResourceManager();
private:
	virtual ~PackageResourceManager() = default;

protected:
	void Init() override;
	void Deinit() override;

	// functionality
	//---------------------
public:

	bool GetLoadData(I_Asset const* const asset, std::vector<uint8>& outData) const override;

	void Flush() override;

	// utility
	//---------------------
protected:
	I_Asset* GetAssetInternal(T_Hash const assetId, std::type_info const& type) override;

	// Data
	///////

	AssetDatabase m_Database;
	std::vector<T_IndexedPackage> m_Packages;
};

