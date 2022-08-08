#pragma once
#include "MaterialInterface.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Util/LinkerUtils.h>

#include <EtRHI/GraphicsTypes/Shader.h>


namespace et {
	REGISTRATION_NS(render);
namespace render {
	class MaterialDescriptor;
} }


namespace et {
namespace render {


//---------------------------------
// Material
//
// Combines with a mesh to create a surface, sets the state of a shader
//
class Material final : public I_Material
{
	// definitions
	//---------------------
	REGISTRATION_FRIEND_NS(render)
	RTTR_ENABLE(I_Material)

public:
	//---------------------------------
	// E_DrawType
	//
	// The type of surface this material represents
	//
	enum class E_DrawType
	{
		Opaque,
		AlphaBlend,
		Custom,
		// AlphaTest
		// Unlit
		// Subsurface
		// Decal
		// Translucent
	};

	// Construct destruct
	//---------------------
private:
	Material() = default;
public:
	Material(AssetPtr<rhi::ShaderData> const shader, 
		E_DrawType const drawType,
		rhi::T_ParameterBlock const defaultParameters, 
		std::vector<AssetPtr<rhi::TextureData>> const& textureReferences);
	~Material();

	// material interface
	//---------------------
	Material const* GetBaseMaterial() const override { return this; }
	rhi::T_ConstParameterBlock GetParameters() const override { return m_DefaultParameters; }

	// accessors
	//---------------------
	rhi::ShaderData const* GetShader() const { return m_Shader.get(); }
	AssetPtr<rhi::ShaderData> GetShaderAsset() const { return m_Shader; }

	E_DrawType GetDrawType() const { return m_DrawType; }

	rhi::T_VertexFlags GetLayoutFlags() const { return m_LayoutFlags; }
	std::vector<int32> const& GetAttributeLocations() const { return m_AttributeLocations; }

	// Data
	///////
private:

	// shading
	E_DrawType m_DrawType = E_DrawType::Opaque;
	AssetPtr<rhi::ShaderData> m_Shader;

	// vertices
	rhi::T_VertexFlags m_LayoutFlags = 0u;
	std::vector<int32> m_AttributeLocations;

	// parameters
	rhi::T_ParameterBlock m_DefaultParameters = nullptr;

	// utility
	std::vector<AssetPtr<rhi::TextureData>> m_TextureReferences; // prevent textures from unloading
};


//---------------------------------
// MaterialAsset
//
// Loadable Material Data
//
class MaterialAsset final : public core::Asset<Material, false>
{
	// definitions
	//-------------
	RTTR_ENABLE(core::Asset<Material, false>)
	DECLARE_FORCED_LINKING()

public:
	static Material* CreateMaterial(std::vector<I_Asset::Reference> const& references, 
		MaterialDescriptor const& descriptor, 
		Material::E_DrawType const drawType);

	// Construct destruct
	//---------------------
	MaterialAsset() : core::Asset<Material, false>() {}
	virtual ~MaterialAsset() = default;

	// Asset interface
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Data
	///////
public:
	Material::E_DrawType m_DrawType = Material::E_DrawType::Opaque;
};


} // namespace render
} // namespace et

