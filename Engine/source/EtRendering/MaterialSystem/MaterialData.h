#pragma once
#include "MaterialInterface.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Helper/LinkerUtils.h>

#include <EtRendering/GraphicsTypes/Shader.h>


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
	Material(AssetPtr<ShaderData> const shader, 
		E_DrawType const drawType,
		T_ParameterBlock const defaultParameters, 
		std::vector<AssetPtr<TextureData>> const& textureReferences);
	~Material();

	// material interface
	//---------------------
	Material const* GetBaseMaterial() const override { return this; }
	T_ConstParameterBlock GetParameters() const override { return m_DefaultParameters; }

	// accessors
	//---------------------
	ShaderData const* GetShader() const { return m_Shader.get(); }
	E_DrawType GetDrawType() const { return m_DrawType; }

	T_VertexFlags GetLayoutFlags() const { return m_LayoutFlags; }
	std::vector<int32> const& GetAttributeLocations() const { return m_AttributeLocations; }

	// Data
	///////
private:

	// shading
	E_DrawType m_DrawType = E_DrawType::Opaque;
	AssetPtr<ShaderData> m_Shader;

	// vertices
	T_VertexFlags m_LayoutFlags = 0u;
	std::vector<int32> m_AttributeLocations;

	// parameters
	T_ParameterBlock m_DefaultParameters = nullptr;

	// utility
	std::vector<AssetPtr<TextureData>> m_TextureReferences; // prevent textures from unloading
};


//---------------------------------
// MaterialAsset
//
// Loadable Material Data
//
class MaterialAsset final : public Asset<Material, false>
{
	// definitions
	//-------------
	RTTR_ENABLE(Asset<Material, false>)
	DECLARE_FORCED_LINKING()

	// Construct destruct
	//---------------------
public:
	MaterialAsset() : Asset<Material, false>() {}
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

