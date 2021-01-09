#pragma once
#include <EtRendering/MaterialSystem/MaterialData.h>
#include <EtRendering/MaterialSystem/MaterialInstance.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//--------------------------------
// EditableMaterialAsset
//
class EditableMaterialAsset final : public EditorAsset<render::Material>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::Material>)
public:
	// Construct destruct
	//---------------------
	EditableMaterialAsset() : EditorAsset<render::Material>() {}
	virtual ~EditableMaterialAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	bool GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath) override;
	bool GenerateRequiresLoadData() const override { return true; }
};


//--------------------------------
// EditableMaterialInstanceAsset
//
class EditableMaterialInstanceAsset final : public EditorAsset<render::MaterialInstance>
{
	RTTR_ENABLE(EditorAsset<render::MaterialInstance>)
public:
	// Construct destruct
	//---------------------
	EditableMaterialInstanceAsset() : EditorAsset<render::MaterialInstance>() {}
	virtual ~EditableMaterialInstanceAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	bool GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath) override;
	bool GenerateRequiresLoadData() const override { return true; }
};


} // namespace pl
} // namespace et
