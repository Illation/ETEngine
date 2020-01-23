#pragma once
#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Content/AssetDatabase.h>


// fwd
namespace et { namespace core {
	class I_Package;
} }


namespace et {
namespace rt {


//---------------------------------
// PackageResourceManager
//
// Resource manager implementation that uses packages
//
class PackageResourceManager : public core::ResourceManager 
{
	// Definitions
	//---------------------
public:
	friend class core::ResourceManager;
	typedef std::pair<T_Hash, core::I_Package*> T_IndexedPackage;

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

	bool GetLoadData(core::I_Asset const* const asset, std::vector<uint8>& outData) const override;

	void Flush() override;

	// utility
	//---------------------
protected:
	core::I_Asset* GetAssetInternal(T_Hash const assetId, std::type_info const& type, bool const reportErrors) override;

	// Data
	///////

	core::AssetDatabase m_Database;
	std::vector<T_IndexedPackage> m_Packages;
};


} // namespace rt
} // namespace et
