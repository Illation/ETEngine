#include "stdafx.h"
#include "MaterialDescriptor.h"

#include <EtRendering/GraphicsTypes/Shader.h>


namespace et {
namespace render {


// reflection
//////////////////

// definition for a parameter deriving from BaseMaterialParam -> registers the type and a convesion function to enable serialized polymorphism
#define REGISTER_DERIVED_PARAM(TClass, TName)					\
	BEGIN_REGISTER_POLYMORPHIC_CLASS(TClass, TName " param")	\
		.property("data", &TClass::m_Data)						\
	END_REGISTER_POLYMORPHIC_CLASS(TClass, BaseMaterialParam);			


// register all the types
RTTR_REGISTRATION
{
	// base
	rttr::registration::class_<BaseMaterialParam>("base material param")
		.property("name", &BaseMaterialParam::GetName, &BaseMaterialParam::SetName);

	// derived
	REGISTER_DERIVED_PARAM(MaterialParam<std::string>, "texture"); // convert string to texture asset reference
	REGISTER_DERIVED_PARAM(MaterialParam<mat4>, "matrix4x4");
	REGISTER_DERIVED_PARAM(MaterialParam<mat3>, "matrix3x3");
	REGISTER_DERIVED_PARAM(MaterialParam<vec4>, "vector4D");
	REGISTER_DERIVED_PARAM(MaterialParam<vec3>, "vector3D");
	REGISTER_DERIVED_PARAM(MaterialParam<vec2>, "vector2D");
	REGISTER_DERIVED_PARAM(MaterialParam<uint32>, "uint");
	REGISTER_DERIVED_PARAM(MaterialParam<int32>, "int");
	REGISTER_DERIVED_PARAM(MaterialParam<float>, "float");
	REGISTER_DERIVED_PARAM(MaterialParam<bool>, "boolean");

	// container
	rttr::registration::class_<MaterialDescriptor>("material descriptor")
		.property("parameters", &MaterialDescriptor::parameters);
}


//=========================
// Base Material Parameter
//=========================


//---------------------------------
// BaseMaterialParam::SetName
//
void BaseMaterialParam::SetName(std::string const& val)
{
	m_Name = val;
	m_Id = GetHash(m_Name);
}


namespace parameters {


//=========================
// parameters
//=========================


//---------------------------------
// ConvertDescriptor
//
// Convert a material descriptor into a shader parameter set
//
void ConvertDescriptor(T_ParameterBlock const baseParams, 
	MaterialDescriptor const& desc, 
	ShaderData const* const shader, 
	std::vector<AssetPtr<TextureData>> const& textureRefs)
{
	ET_ASSERT(shader != nullptr);

	std::vector<UniformParam> const& layout = shader->GetUniformLayout();
	std::vector<T_Hash> const& ids = shader->GetUniformIds();
	ET_ASSERT(layout.size() == ids.size());

	// override all 
	for (size_t paramIdx = 0u; paramIdx < layout.size(); ++paramIdx)
	{
		UniformParam const& param = layout[paramIdx];
		T_Hash const paramId = ids[paramIdx];

		// try finding the corresponding parameter in the descriptor
		auto paramIt = std::find_if(desc.parameters.cbegin(), desc.parameters.cend(), [paramId](BaseMaterialParam const* const baseParam)
			{
				return baseParam->GetId() == paramId;
			});

		// if we don't find it that's fine
		if (paramIt == desc.parameters.cend())
		{
			continue;
		}

		BaseMaterialParam const* const baseParam = *paramIt;
		switch (param.type)
		{
		case E_ParamType::Texture2D:
		case E_ParamType::Texture3D:
		case E_ParamType::TextureCube:
		case E_ParamType::TextureShadow:
			{
				ET_ASSERT(baseParam->GetType() == typeid(std::string));
				T_Hash const assetId = GetHash(static_cast<MaterialParam<std::string> const*>(baseParam)->GetData());

				auto texIt = std::find_if(textureRefs.cbegin(), textureRefs.cend(), [assetId](AssetPtr<TextureData> const& texture)
					{
						return texture.GetAsset()->GetId() == assetId;
					});

				if (texIt == textureRefs.cend())
				{
					ET_ASSERT(false, "Unreferenced texture parameter!");
					break;
				}

				render::parameters::Write(baseParams, param.offset, (*texIt).get());
			}
			break;

		case E_ParamType::Matrix4x4:
			ET_ASSERT(baseParam->GetType() == typeid(mat4));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<mat4> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Matrix3x3:
			ET_ASSERT(baseParam->GetType() == typeid(mat3));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<mat3> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Vector4:
			ET_ASSERT(baseParam->GetType() == typeid(vec4));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<vec4> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Vector3:
			ET_ASSERT(baseParam->GetType() == typeid(vec3));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<vec3> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Vector2:
			ET_ASSERT(baseParam->GetType() == typeid(vec2));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<vec2> const*>(baseParam)->GetData());
			break;

		case E_ParamType::UInt:
			ET_ASSERT(baseParam->GetType() == typeid(uint32));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<uint32> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Int:
			ET_ASSERT(baseParam->GetType() == typeid(int32));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<int32> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Float:
			ET_ASSERT(baseParam->GetType() == typeid(float));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<float> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Boolean:
			ET_ASSERT(baseParam->GetType() == typeid(bool));
			parameters::Write(baseParams, param.offset, static_cast<MaterialParam<bool> const*>(baseParam)->GetData());
			break;

		default:
			ET_ASSERT(false, "Unhandled parameter type");
			break;
		}
	}
}


} // namespace parameters

} // namespace render
} // namespace et
