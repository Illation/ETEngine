#include "stdafx.h"
#include "MaterialInstance.h"

#include "MaterialDescriptor.h"

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Reflection/Registration.h>
#include <EtCore/Reflection/BinaryDeserializer.h>


namespace et {
namespace render {


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(MaterialInstance, "material instance")
	END_REGISTER_CLASS_POLYMORPHIC(MaterialInstance, I_Material);

	BEGIN_REGISTER_CLASS(MaterialInstanceAsset, "material instance asset")
	END_REGISTER_CLASS_POLYMORPHIC(MaterialInstanceAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(MaterialInstanceAsset) // force the material instance asset class to be linked


//===================
// Material Instance
//===================


//--------------------------
// MaterialInstance::c-tor
//
// Construct from material
//
MaterialInstance::MaterialInstance(AssetPtr<Material> const material,
	T_ParameterBlock const params, 
	std::vector<AssetPtr<TextureData>> const& textureRefs
)
	: I_Material()
	, m_Material(material)
	, m_Parameters(params)
	, m_TextureReferences(textureRefs)
{ }

//--------------------------
// MaterialInstance::c-tor
//
// Construct from material instance
//
MaterialInstance::MaterialInstance(AssetPtr<MaterialInstance> const parent,
	T_ParameterBlock const params,
	std::vector<AssetPtr<TextureData>> const& textureRefs
)
	: I_Material()
	, m_Parent(parent)
	, m_Parameters(params)
	, m_TextureReferences(textureRefs)
{ 
	ET_ASSERT(m_Parent != nullptr);

	m_Material = m_Parent->GetMaterialAsset();
}

//--------------------------
// MaterialInstance::d-tor
//
MaterialInstance::~MaterialInstance()
{
	parameters::DestroyBlock(m_Parameters);
}


//=========================
// Material Instance Asset
//=========================


//-----------------------------------------------
// MaterialInstanceAsset::CreateMaterialInstance
//
// Create material instance from material descriptor and asset references
//
MaterialInstance* MaterialInstanceAsset::CreateMaterialInstance(std::vector<core::I_Asset::Reference> const& references, 
	MaterialDescriptor const& descriptor)
{
	// extract the material or parent and texture references
	std::vector<AssetPtr<TextureData>> textureRefs;
	AssetPtr<Material> materialRef;
	AssetPtr<MaterialInstance> parentRef;
	ShaderData const* shader;

	for (core::I_Asset::Reference const& reference : references)
	{
		I_AssetPtr const* const rawAssetPtr = reference.GetAsset();

		if (rawAssetPtr->GetType() == rttr::type::get<Material>())
		{
			ET_ASSERT(materialRef == nullptr && parentRef == nullptr, "Material Instances can only reference one material or parent!");
			materialRef = *static_cast<AssetPtr<Material> const*>(rawAssetPtr);
			shader = materialRef->GetShader();
		}
		else if (rawAssetPtr->GetType() == rttr::type::get<MaterialInstance>())
		{
			ET_ASSERT(materialRef == nullptr && parentRef == nullptr, "Material Instances can only reference one material or parent!");
			parentRef = *static_cast<AssetPtr<MaterialInstance> const*>(rawAssetPtr);
			shader = parentRef->GetMaterialAsset()->GetShader();
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

	if ((materialRef == nullptr) && (parentRef == nullptr))
	{
		ET_LOG_W(ET_CTX_RENDER, "MaterialAsset::LoadFromMemory > Material instances must reference a material or parent instance!");
		return nullptr;
	}

	// override the parents parameters with the parameter descriptor
	T_ParameterBlock const params = shader->CopyParameterBlock((parentRef != nullptr) ? parentRef->GetParameters() : materialRef->GetParameters());
	if (params != nullptr)
	{
		parameters::ConvertDescriptor(params, descriptor, shader, textureRefs);
	}

	// Create the material instance
	if (parentRef != nullptr)
	{
		return new MaterialInstance(parentRef, params, textureRefs);
	}
	else
	{
		return new MaterialInstance(materialRef, params, textureRefs);
	}
}

//---------------------------------------
// MaterialInstanceAsset::LoadFromMemory
//
// Load material instance data from binary asset content
//
bool MaterialInstanceAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	MaterialDescriptor descriptor;
	core::BinaryDeserializer deserializer;
	if (!deserializer.DeserializeFromData(data, descriptor))
	{
		ET_LOG_W(ET_CTX_RENDER, "MaterialAsset::LoadFromMemory > Failed to deserialize data from a JSON format into a material descriptor");
		return false;
	}

	m_Data = CreateMaterialInstance(GetReferences(), descriptor);
	return (m_Data != nullptr);
}


} // namespace render
} // namespace et
