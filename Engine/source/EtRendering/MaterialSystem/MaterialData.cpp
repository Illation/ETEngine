#include "stdafx.h"
#include "MaterialData.h"

#include "MaterialDescriptor.h"

#include <EtCore/Reflection/Serialization.h>
#include <EtCore/FileSystem/FileUtil.h>


// reflection
//////////////
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::enumeration<render::Material::E_DrawType>("E_DrawType") (
		value("Opaque", render::Material::E_DrawType::Opaque),
		value("AlphaBlend", render::Material::E_DrawType::AlphaBlend));

	registration::class_<render::MaterialAsset>("material asset")
		.constructor<render::MaterialAsset const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("draw type", &render::MaterialAsset::m_DrawType);

	rttr::type::register_converter_func([](render::MaterialAsset& material, bool& ok) -> I_Asset*
	{
		ok = true;
		return new render::MaterialAsset(material);
	});
}
DEFINE_FORCED_LINKING(render::MaterialAsset) // force the material asset class to be linked


namespace render {


//==========
// Material 
//==========


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


//---------------------------------
// MaterialAsset::LoadFromMemory
//
// Load shader data from binary asset content
//
bool MaterialAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	MaterialDescriptor descriptor;
	if (!(serialization::DeserializeFromJsonString(FileUtil::AsText(data), descriptor)))
	{
		LOG("MaterialAsset::LoadFromMemory > Failed to deserialize data from a JSON format into a material descriptor", LogLevel::Warning);
		return false;
	}

	// extract the shader and texture references
	std::vector<AssetPtr<TextureData>> textureRefs;
	AssetPtr<ShaderData> shaderRef;

	for (I_Asset::Reference const& reference : GetReferences())
	{
		I_AssetPtr const* const rawAssetPtr = reference.GetAsset();

		if (rawAssetPtr->GetType() == typeid(ShaderData))
		{
			ET_ASSERT(shaderRef == nullptr, "Materials cannot reference more than one shader!");
			shaderRef = *static_cast<AssetPtr<ShaderData> const*>(rawAssetPtr);
		}
		else if (rawAssetPtr->GetType() == typeid(TextureData))
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
		LOG("MaterialAsset::LoadFromMemory > Materials must reference a shader!", LogLevel::Warning);
		return false;
	}

	// convert to a parameter block
	// ideally this should be done before any uniforms are set in the shader
	T_ParameterBlock const params = shaderRef->CopyParameterBlock(shaderRef->GetCurrentUniforms());
	if (params != nullptr)
	{
		parameters::ConvertDescriptor(params, descriptor, shaderRef.get(), textureRefs);
	}

	// Create the material
	m_Data = new Material(shaderRef, m_DrawType, params, textureRefs);
	return true;
}


} // namespace render

