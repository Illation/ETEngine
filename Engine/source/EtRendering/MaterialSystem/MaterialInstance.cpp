#include "stdafx.h"
#include "MaterialInstance.h"

#include "MaterialDescriptor.h"

#include <EtCore/Reflection/Serialization.h>
#include <EtCore/FileSystem/FileUtil.h>


// reflection
//////////////
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<render::MaterialInstanceAsset>("material instance asset")
		.constructor<render::MaterialInstanceAsset const&>()
		.constructor<>()(rttr::detail::as_object());

	rttr::type::register_converter_func([](render::MaterialInstanceAsset& materialInstance, bool& ok) -> I_Asset*
	{
		ok = true;
		return new render::MaterialInstanceAsset(materialInstance);
	});
}
DEFINE_FORCED_LINKING(render::MaterialInstanceAsset) // force the material instance asset class to be linked


namespace render {


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


//---------------------------------------
// MaterialInstanceAsset::LoadFromMemory
//
// Load material instance data from binary asset content
//
bool MaterialInstanceAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	MaterialDescriptor descriptor;
	if (!(serialization::DeserializeFromJsonString(FileUtil::AsText(data), descriptor)))
	{
		LOG("MaterialAsset::LoadFromMemory > Failed to deserialize data from a JSON format into a material descriptor", LogLevel::Warning);
		return false;
	}

	// extract the material or parent and texture references
	std::vector<AssetPtr<TextureData>> textureRefs;
	AssetPtr<Material> materialRef;
	AssetPtr<MaterialInstance> parentRef;
	ShaderData const* shader;

	for (I_Asset::Reference const& reference : GetReferences())
	{
		I_AssetPtr const* const rawAssetPtr = reference.GetAsset();

		if (rawAssetPtr->GetType() == typeid(Material))
		{
			ET_ASSERT(materialRef == nullptr && parentRef == nullptr, "Material Instances can only reference one material or parent!");
			materialRef = *static_cast<AssetPtr<Material> const*>(rawAssetPtr);
			shader = materialRef->GetShader();
		}
		else if (rawAssetPtr->GetType() == typeid(MaterialInstance))
		{
			ET_ASSERT(materialRef == nullptr && parentRef == nullptr, "Material Instances can only reference one material or parent!");
			parentRef = *static_cast<AssetPtr<MaterialInstance> const*>(rawAssetPtr);
			shader = parentRef->GetMaterialAsset()->GetShader();
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

	if ((materialRef == nullptr) && (parentRef == nullptr))
	{
		LOG("MaterialAsset::LoadFromMemory > Material instances must reference a material or parent instance!", LogLevel::Warning);
		return false;
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
		m_Data = new MaterialInstance(parentRef, params, textureRefs);
	}
	else
	{
		m_Data = new MaterialInstance(materialRef, params, textureRefs);
	}

	return true;
}


} // namespace render

