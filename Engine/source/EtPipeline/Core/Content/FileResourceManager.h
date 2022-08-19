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
public:
	friend class core::ResourceManager;

	// Construct destruct
	//---------------------
	FileResourceManager(std::string const& projectDbPath, std::string const& engineDbPath);
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
	void SetLoadEnabled(bool const val) { m_IsLoadEnabled = val; } // DANGERZONE - allow disabling asset load during data generation

	// accessors
	//-----------
	EditorAssetDatabase& GetProjectDatabase() { return m_ProjectDb; }
	EditorAssetDatabase& GetEngineDatabase() { return m_EngineDb; }

	bool IsLoadEnabled() const { return m_IsLoadEnabled; } 

	EditorAssetDatabase const* GetDB(core::I_Asset const* const asset) const;

	// utility
	//---------------------
protected:
	core::I_Asset* GetAssetInternal(core::HashString const assetId, rttr::type const type, bool const reportErrors) override;
	EditorAssetBase* GetEditorAsset(core::HashString const assetId, rttr::type const type, bool const reportErrors);
	EditorAssetBase* GetEditorAsset(core::I_Asset* const asset, bool const reportErrors = true);

	bool IsEngineResource(core::I_Asset const* const asset) const;

	// Data
	///////

	std::string m_ProjectDbPath;
	EditorAssetDatabase m_ProjectDb;

	std::string m_EngineDbPath;
	EditorAssetDatabase m_EngineDb;

	bool m_IsLoadEnabled = true;
};


} // namespace pl
} // namespace et
