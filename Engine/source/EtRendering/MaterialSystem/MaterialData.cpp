#include "stdafx.h"
#include "MaterialData.h"

#include "MaterialDescriptor.h"

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Reflection/BinaryDeserializer.h>


namespace et {
namespace render {


//==========
// Material 
//==========


// reflection
//////////////
RTTR_REGISTRATION
{
	rttr::registration::enumeration<Material::E_DrawType>("E_DrawType") (
		rttr::value("Opaque", Material::E_DrawType::Opaque),
		rttr::value("AlphaBlend", Material::E_DrawType::AlphaBlend),
		rttr::value("Custom", Material::E_DrawType::Custom));

	REGISTER_CLASS_ASSET_BASE(I_Material, "material interface");

	BEGIN_REGISTER_CLASS_ASSET(Material, "material")
	END_REGISTER_CLASS_POLYMORPHIC(Material, I_Material);

	BEGIN_REGISTER_CLASS(MaterialAsset, "material asset")
		.property("draw type", &MaterialAsset::m_DrawType)
	END_REGISTER_CLASS_POLYMORPHIC(MaterialAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(MaterialAsset) // force the material asset class to be linked


//--------------------------
// Material::c-tor
//
// Construct material, set up vertex info etc
//
Material::Material(AssetPtr<ShaderData> const shader, 
	E_DrawType const drawType, 
	T_ParameterBlock const defaultParameters, 
	std::vector<AssetPtr<TextureData>> const& textureReferences
)
	: I_Material()
	, m_Shader(shader)
	, m_DrawType(drawType)
	, m_DefaultParameters(defaultParameters)
	, m_TextureReferences(textureReferences)
{
	ET_ASSERT(m_Shader != nullptr);

	// determine layout flags and locations
	std::vector<ShaderData::T_AttributeLocation> const& attributes = m_Shader->GetAttributes();

	for (auto it = AttributeDescriptor::s_VertexAttributes.begin(); it != AttributeDescriptor::s_VertexAttributes.end(); ++it)
	{
		auto const attribIt = std::find_if(attributes.cbegin(), attributes.cend(), [it](ShaderData::T_AttributeLocation const& loc)
		{
			return it->second.name == loc.second.name;
		});

		if (attribIt != attributes.cend())
		{
			m_AttributeLocations.emplace_back(attribIt->first);
			m_LayoutFlags |= it->first;
		}
	}
}

//--------------------------
// Material::d-tor
//
Material::~Material()
{
	if (m_DefaultParameters != nullptr)
	{
		parameters::DestroyBlock(m_DefaultParameters);
	}
}


//================
// Material Asset
//================


//-------------------------------
// MaterialAsset::CreateMaterial
//
// Create a material from a descriptor and a set of asset references
//
Material* MaterialAsset::CreateMaterial(std::vector<I_Asset::Reference> const& references, 
	MaterialDescriptor const& descriptor, 
	Material::E_DrawType const drawType)
{
	// extract the shader and texture references
	std::vector<AssetPtr<TextureData>> textureRefs;
	AssetPtr<ShaderData> shaderRef;

	for (I_Asset::Reference const& reference : references)
	{
		I_AssetPtr const* const rawAssetPtr = reference.GetAsset();

		if (rawAssetPtr->GetType() == rttr::type::get<ShaderData>())
		{
			ET_ASSERT(shaderRef == nullptr, "Materials cannot reference more than one shader!");
			shaderRef = *static_cast<AssetPtr<ShaderData> const*>(rawAssetPtr);
		}
		else if (rawAssetPtr->GetType() == rttr::type::get<TextureData>())
		{
			textureRefs.push_back(*static_cast<AssetPtr<TextureData> const*>(rawAssetPtr));
		}
		else
		{
			ET_ASSERT(false, "unhandled reference type!");
		}
	}

	if (shaderRef == nullptr)
	{
		LOG("MaterialAsset::LoadFromMemory > Materials must reference a shader!", core::LogLevel::Warning);
		return nullptr;
	}

	// convert to a parameter block
	// ideally this should be done before any uniforms are set in the shader
	T_ParameterBlock const params = shaderRef->CopyParameterBlock(shaderRef->GetCurrentUniforms());
	if (params != nullptr)
	{
		parameters::ConvertDescriptor(params, descriptor, shaderRef.get(), textureRefs);
	}

	// Create the material
	return new Material(shaderRef, drawType, params, textureRefs);
}

//---------------------------------
// MaterialAsset::LoadFromMemory
//
// Load material data from binary asset content
//
bool MaterialAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	MaterialDescriptor descriptor;
	core::BinaryDeserializer deserializer;
	if (!deserializer.DeserializeFromData(data, descriptor))
	{
		LOG("MaterialAsset::LoadFromMemory > Failed to deserialize data from a JSON format into a material descriptor", core::LogLevel::Warning);
		return false;
	}

	m_Data = CreateMaterial(GetReferences(), descriptor, m_DrawType);
	return (m_Data != nullptr);
}


} // namespace render
} // namespace et
