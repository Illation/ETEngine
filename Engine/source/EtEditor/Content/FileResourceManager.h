#pragma once
#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Content/AssetDatabase.h>


// forward decl
namespace et { namespace core {
	class I_Package;
	class Directory;
} }


namespace et {
namespace edit {


//---------------------------------
// FileResourceManager
//
// Resource manager implementation that uses packages
//
class FileResourceManager : public core::ResourceManager
{
	// Definitions
	//---------------------
	static constexpr char s_ResourceDirRelPath[] = "resources/";

public:
	friend class core::ResourceManager;

	// Construct destruct
	//---------------------
	FileResourceManager();
private:
	virtual ~FileResourceManager() = default;

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
	core::I_Asset* GetAssetInternal(core::HashString const assetId, std::type_info const& type, bool const reportErrors) override;

	void InitDb(core::AssetDatabase& db, core::Directory*& dir, std::string const& path);

	bool IsEngineResource(core::I_Asset const* const asset) const;

	// Data
	///////

	core::AssetDatabase m_ProjectDb;
	core::Directory* m_ProjectDir = nullptr;

	core::AssetDatabase m_EngineDb;
	core::Directory* m_EngineDir = nullptr;
};


} // namespace edit
} // namespace et
