#pragma once
#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Content/AssetDatabase.h>


// forward decl
class I_Package;
class Directory;


namespace et {
namespace edit {


//---------------------------------
// FileResourceManager
//
// Resource manager implementation that uses packages
//
class FileResourceManager : public ResourceManager
{
	// Definitions
	//---------------------
	static constexpr char s_ResourceDirRelPath[] = "resources/";

public:
	friend class ResourceManager;

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

	bool GetLoadData(I_Asset const* const asset, std::vector<uint8>& outData) const override;

	void Flush() override;

	// utility
	//---------------------
protected:
	I_Asset* GetAssetInternal(T_Hash const assetId, std::type_info const& type, bool const reportErrors) override;

	void InitDb(AssetDatabase& db, Directory*& dir, std::string const& path);

	bool IsEngineResource(I_Asset const* const asset) const;

	// Data
	///////

	AssetDatabase m_ProjectDb;
	Directory* m_ProjectDir = nullptr;

	AssetDatabase m_EngineDb;
	Directory* m_EngineDir = nullptr;
};


} // namespace edit
} // namespace et
