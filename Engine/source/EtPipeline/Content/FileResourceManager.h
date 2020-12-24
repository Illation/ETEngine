#pragma once
#include <EtCore/Content/ResourceManager.h>

#include "EditorAssetDatabase.h"


namespace et {
namespace pl {


//---------------------------------
// FileResourceManager
//
// Resource manager implementation that handles assets in editor friendly format (loose, text based, with additional meta data)
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
	FileResourceManager(std::string const& projectPath, std::string const& enginePath);
private:
	virtual ~FileResourceManager() = default;

protected:
	void Init() override;
	void Deinit() override {}

	// functionality
	//---------------------

	void LoadAsset(core::I_Asset* const asset) override; 
	void UnloadAsset(core::I_Asset* const asset) override;

public:
	bool GetLoadData(core::I_Asset const* const asset, std::vector<uint8>& outData) const override;

	void Flush() override;

	// accessors
	//-----------
	EditorAssetDatabase& GetProjectDatabase() { return m_ProjectDb; }
	EditorAssetDatabase& GetEngineDatabase() { return m_EngineDb; }

	// utility
	//---------------------
protected:
	core::I_Asset* GetAssetInternal(core::HashString const assetId, rttr::type const type, bool const reportErrors) override;
	EditorAssetBase* GetEditorAsset(core::HashString const assetId, rttr::type const type, bool const reportErrors);
	EditorAssetBase* GetEditorAsset(core::I_Asset* const asset, bool const reportErrors = true);

	void InitDb(EditorAssetDatabase& db, std::string const& path);

	bool IsEngineResource(core::I_Asset const* const asset) const;

	// Data
	///////

	std::string m_ProjectPath;
	EditorAssetDatabase m_ProjectDb;

	std::string m_EnginePath;
	EditorAssetDatabase m_EngineDb;
};


} // namespace pl
} // namespace et
