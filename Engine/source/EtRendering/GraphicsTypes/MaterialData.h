#pragma once
#include "Shader.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Helper/LinkerUtils.h>


namespace render {


//---------------------------------
// Material
//
// Combines with a mesh to create a surface, sets the state of a shader
//
class Material final
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

	// accessors
	//---------------------
	ShaderData const* GetShader() const { return m_Shader.get(); }
	E_DrawType GetDrawType() const { return m_DrawType; }
	T_VertexFlags GetLayoutFlags() const { return m_LayoutFlags; }
	std::vector<int32> const& GetAttributeLocations() const { return m_AttributeLocations; }

	// functionliaty
	//---------------------
	void UploadToShader() const;

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
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	MaterialAsset() : Asset<Material, false>() {}
	virtual ~MaterialAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Data
	///////
public:
	Material::E_DrawType m_DrawType = Material::E_DrawType::Opaque;

	RTTR_ENABLE(Asset<Material, false>)
};


} // namespace render

