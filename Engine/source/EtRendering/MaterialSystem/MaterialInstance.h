#pragma once
#include "MaterialData.h"


namespace et {
namespace render {


//---------------------------------
// MaterialInstance
//
// Child of a material or another material instance, can override material rhi::parameters
//
class MaterialInstance final : public I_Material
{
	// definitions
	//---------------------
	REGISTRATION_FRIEND_NS(render)
	RTTR_ENABLE(I_Material)

	// Construct destruct
	//---------------------
	MaterialInstance() = default;
public:
	MaterialInstance(AssetPtr<Material> const material, rhi::T_ParameterBlock const params, std::vector<AssetPtr<rhi::TextureData>> const& textureRefs);
	MaterialInstance(AssetPtr<MaterialInstance> const parent, rhi::T_ParameterBlock const params, std::vector<AssetPtr<rhi::TextureData>> const& textureRefs);
	~MaterialInstance();

	// material interface
	//---------------------
	Material const* GetBaseMaterial() const override { return m_Material.get(); }
	rhi::T_ConstParameterBlock GetParameters() const override { return m_Parameters; }

	// accessors
	//---------------------
	AssetPtr<Material> GetMaterialAsset() const { return m_Material; }
	MaterialInstance const* GetParent() const { return m_Parent.get(); }

	// Data
	///////
private:

	// shading
	AssetPtr<Material> m_Material;
	AssetPtr<MaterialInstance> m_Parent;

	// parameters
	rhi::T_ParameterBlock m_Parameters = nullptr;

	// utility
	std::vector<AssetPtr<rhi::TextureData>> m_TextureReferences; // prevent textures from unloading
};


//---------------------------------
// MaterialInstanceAsset
//
// Loadable material instance Data
//
class MaterialInstanceAsset final : public core::Asset<MaterialInstance, false>
{
	// definitions
	//-------------
	RTTR_ENABLE(core::Asset<MaterialInstance, false>)
	DECLARE_FORCED_LINKING()

public:
	static MaterialInstance* CreateMaterialInstance(std::vector<core::I_Asset::Reference> const& references, MaterialDescriptor const& descriptor);

	// Construct destruct
	//---------------------
	MaterialInstanceAsset() : core::Asset<MaterialInstance, false>() {}
	virtual ~MaterialInstanceAsset() = default;

	// Asset interface
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;
};


} // namespace render
} // namespace et

